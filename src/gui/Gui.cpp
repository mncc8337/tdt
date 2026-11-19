#include "Gui.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <map>

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#endif

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-SFML.h"
#include "nfd.hpp"

#include "misc/ColorUtils.h"
#include "rendering/Texture.h"
#include "rendering/texture/SolidTexture.h"
#include "rendering/Material.h"
#include "rendering/material/Metal.h"
#include "rendering/material/Matte.h"
#include "rendering/material/Dielectric.h"
#include "rendering/material/Light.h"
#include "rendering/Medium.h"
#include "rendering/medium/Homogeneous.h"
#include "rendering/hittable/Mesh.h"
#include "rendering/hittable/Sphere.h"

static std::string toLower(std::string text) {
    std::transform(
        text.begin(),
        text.end(),
        text.begin(),
        [](unsigned char c) { return (char)std::tolower(c); }
    );
    return text;
}

static sf::VideoMode initialWindowSize() {
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

    unsigned width = std::min(1600u, desktop.size.x);
    unsigned height = std::min(900u, desktop.size.y);

    if(desktop.size.x > 200) width = std::min(width, desktop.size.x - 100);
    if(desktop.size.y > 200) height = std::min(height, desktop.size.y - 100);

    return sf::VideoMode({
        std::max(width, 640u),
        std::max(height, 480u)
    });
}

Gui::Gui(Camera& camera, Scene& scene, unsigned width, unsigned height):
    camera(camera),
    scene(scene),
    tracer(width, height),
    window(initialWindowSize(), "tdt"),
    texture(sf::Vector2u(width, height)),
    tracer_buffer_copy(width * height),
    display_pixels(width * height)
{
    if(!ImGui::SFML::Init(window)) {
        throw std::runtime_error("Failed to initialize imgui.");
    }

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    window.setFramerateLimit(60);
}

Gui::~Gui() {
    is_gui_running = false;
    if(render_thread.joinable()) {
        render_thread.join();
    }
    ImGui::SFML::Shutdown();
}

std::string Gui::getTypeName(const std::type_info& ti) {
#if defined(__GNUC__) || defined(__clang__)
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> res{
        abi::__cxa_demangle(ti.name(), nullptr, nullptr, &status),
        std::free
    };
    return (status == 0) ? res.get() : ti.name();
#else
    return ti.name();
#endif
}

void Gui::postAction(std::function<void()> action) {
    pending_actions.push_back(std::move(action));
    lock_requested = true;
}

void Gui::applyPendingActions() {
    if(pending_actions.empty()) return;

    std::unique_lock<std::mutex> lock(scene_mutex, std::try_to_lock);

    // only apply changes when the tracer is done rendered
    // i.e the scene is not locked
    if(!lock.owns_lock()) return;

    for(const auto& edit : pending_actions) {
        edit();
    }
    pending_actions.clear();

    if(is_geometry_dirty) {
        scene.buildBVH();
        is_geometry_dirty = false;
    }

    // the scene has changed
    // so reset the accumulated result
    tracer.reset();
    // the limit refers to the previously rendered frame count, and the
    // displayed image belongs to the old scene: drop both
    frame_limit_reached = false;
    clearDisplayedFrame();
    total_time = 0;
    last_dt = 0;

    lock_requested = false;
}

void Gui::clearDisplayedFrame() {
    std::fill(tracer_buffer_copy.begin(), tracer_buffer_copy.end(), Color(0));
    front_pass = 0;
    is_data_ready = true;
}

void Gui::updateRenderState() {
    is_rendering_paused = manual_pause || frame_limit_reached.load();
}

bool Gui::save(const std::filesystem::path &filename) {
    // take a stable snapshot, then resolve on the GUI thread so saving does
    // not add post-processing work to the render loop
    std::vector<Color> buffer;
    unsigned pass;
    unsigned width;
    unsigned height;
    {
        std::lock_guard<std::mutex> lock(scene_mutex);
        buffer = tracer.getBuffer();
        pass = tracer.getPass();
        width = tracer.getViewportWidth();
        height = tracer.getViewportHeight();
    }

    std::vector<std::uint32_t> pixels;
    resolvePixels(buffer, pixels, pass, exposure, gamma, tonemap);

    sf::Image img(
        sf::Vector2u(width, height),
        reinterpret_cast<const std::uint8_t*>(pixels.data())
    );
    return img.saveToFile(filename);
}

void Gui::saveWithDialog() {
    NFD::Guard nfdGuard;
    NFD::UniquePath outPath;
    nfdfilteritem_t filterItem[] = {{"Image", "png,jpg,bmp,tga"}};

    nfdresult_t result = NFD::SaveDialog(outPath, filterItem, 1);
    if(result == NFD_OKAY) {
        save(outPath.get());
    } else if(result == NFD_CANCEL) {
        // do nothing
    } else {
        printf("Error: %s\n", NFD::GetError());
    }
}

void Gui::renderRoutine() {
    sf::Clock delta_clock;

    while(is_gui_running) {
        // wait when is paused or the gui requested the lock
        while(is_gui_running && (is_rendering_paused || lock_requested)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if(!is_gui_running) break;

        // drop the time spent spinning so it is not counted as render time
        delta_clock.restart();

        {
            std::lock_guard<std::mutex> lock(scene_mutex);

            tracer.render(camera, scene);

            // the frame limit is checked here, right after the pass that
            // reached it, so the render thread stops without another pass
            if(limit_frame && target_frame > 0 &&
               tracer.getPass() >= target_frame) {
                frame_limit_reached = true;
                is_rendering_paused = true;
            }

            // publish only if the GUI consumed the previous frame, so
            // front_buffer is never read while it is being written
            if(!is_data_ready) {
                std::memcpy(
                    tracer_buffer_copy.data(),
                    tracer.getBuffer().data(),
                    tracer_buffer_copy.size() * sizeof(Color)
                );
                front_pass = tracer.getPass();
                is_data_ready = true;
            }
        }

        last_dt = delta_clock.restart().asMilliseconds();
        total_time += last_dt;
    }
}

void Gui::handlePicking() {
    if(!render_view_hovered) return;
    if(!ImGui::IsMouseClicked(ImGuiMouseButton_Left)) return;
    if(render_view_width <= 0.0f || render_view_height <= 0.0f) return;

    ImVec2 mouse = ImGui::GetIO().MousePos;

    // map the mouse position onto the rendered image, which is letterboxed
    // inside its dock window and therefore not the same as the window size
    float u = (mouse.x - render_view_x) / render_view_width;
    float v = (mouse.y - render_view_y) / render_view_height;
    if(u < 0.0f || u >= 1.0f || v < 0.0f || v >= 1.0f) return;

    sf::Vector2u tex_size = texture.getSize();
    if(tex_size.x == 0 || tex_size.y == 0) return;

    float mx = u * (float)tex_size.x;
    float my = v * (float)tex_size.y;

    // reproduce the renderer's viewport mapping with a single
    // camera ray shot through the clicked pixel (1-pass trace)
    float viewport_h = 2.0f;
    float viewport_w = viewport_h * ((float)tex_size.x / (float)tex_size.y);

    float cu = (mx - (float)tex_size.x / 2.0f) / (float)tex_size.x;
    float cv = (my - (float)tex_size.y / 2.0f) / (float)tex_size.y;

    Ray ray = camera.getRayAt(Vec2(cu * viewport_w, -cv * viewport_h), false);
    HitInfo hit = scene.getClosest(ray, selected_object);
    requested_properties_tab = PropertiesTab::Object;
    properties_tab_requested = true;
}

void Gui::drawMainMenuBar() {
    if(!ImGui::BeginMainMenuBar()) return;

    if(ImGui::BeginMenu("File")) {
        if(ImGui::MenuItem("Save render...")) {
            saveWithDialog();
        }
        ImGui::EndMenu();
    }

    if(ImGui::BeginMenu("View")) {
        if(ImGui::MenuItem("Reset layout")) {
            reset_layout_requested = true;
        }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void Gui::drawDockspace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiID dockspace_id = ImGui::GetID("MainDockspace.v2");

    if(!layout_initialized) {
        // check for a saved layout only on the first frame, before this
        // frame's DockSpace call creates an empty node of its own
        if(!layout_checked_saved) {
            layout_checked_saved = true;
            has_saved_layout =
                ImGui::DockBuilderGetNode(dockspace_id) != nullptr;
            if(has_saved_layout) {
                layout_initialized = true;
            }
        }

        if(!layout_initialized) {
            // wait until the window size stops changing before building
            // the default layout, so its proportions match the window the
            // user will actually be looking at
            sf::Vector2u current_size = window.getSize();
            if(current_size == last_window_size) {
                stable_window_frames++;
            } else {
                last_window_size = current_size;
                stable_window_frames = 0;
            }

            if(stable_window_frames >= 2) {
                layout_initialized = true;
                reset_layout_requested = true;
            }
        }
    }

    if(reset_layout_requested) {
        reset_layout_requested = false;

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

        // one large render view on the left, one column on the right
        ImGuiID render_node = dockspace_id;
        ImGuiID side_node = ImGui::DockBuilderSplitNode(
            render_node,
            ImGuiDir_Right,
            0.28f,
            nullptr,
            &render_node
        );

        // the right column holds the outliner on top and the properties
        // tabs below, mirroring the usual blender arrangement
        ImGuiID outliner_node = side_node;
        ImGuiID properties_node = ImGui::DockBuilderSplitNode(
            side_node,
            ImGuiDir_Down,
            0.68f,
            nullptr,
            &outliner_node
        );

        ImGui::DockBuilderDockWindow("Render", render_node);
        ImGui::DockBuilderDockWindow("Outliner", outliner_node);
        ImGui::DockBuilderDockWindow("Properties", properties_node);

        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::DockSpaceOverViewport(dockspace_id, viewport);
}

void Gui::drawRenderWindow() {
    // no close button (p_open is null) so the render view is always available
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(
        "Render",
        nullptr,
        ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse
    );
    ImGui::PopStyleVar();

    ImVec2 available = ImGui::GetContentRegionAvail();
    sf::Vector2u tex_size = texture.getSize();

    if(available.x > 0.0f && available.y > 0.0f && tex_size.x > 0 && tex_size.y > 0) {
        float texture_ratio = (float)tex_size.x / (float)tex_size.y;
        float available_ratio = available.x / available.y;

        sf::Vector2f image_size;
        if(available_ratio > texture_ratio) {
            image_size.y = available.y;
            image_size.x = available.y * texture_ratio;
        } else {
            image_size.x = available.x;
            image_size.y = available.x / texture_ratio;
        }

        // center the letterboxed image inside the dock window
        ImVec2 offset(
            (available.x - image_size.x) * 0.5f,
            (available.y - image_size.y) * 0.5f
        );
        ImGui::SetCursorPos(ImVec2(
            ImGui::GetCursorPosX() + offset.x,
            ImGui::GetCursorPosY() + offset.y
        ));

        ImGui::Image(texture, image_size);

        ImVec2 image_min = ImGui::GetItemRectMin();
        ImVec2 image_max = ImGui::GetItemRectMax();

        render_view_x = image_min.x;
        render_view_y = image_min.y;
        render_view_width = image_max.x - image_min.x;
        render_view_height = image_max.y - image_min.y;
        render_view_hovered = ImGui::IsItemHovered();

        ImGui::GetWindowDrawList()->AddRect(
            image_min,
            image_max,
            IM_COL32(255, 255, 255, 36)
        );
    } else {
        render_view_hovered = false;
        render_view_width = 0.0f;
        render_view_height = 0.0f;
    }

    ImGui::End();
}

void Gui::drawOutlinerWindow() {
    ImGui::Begin("Outliner");

    if(ImGui::BeginTabBar("##outliner_tabs", ImGuiTabBarFlags_Reorderable)) {
        if(ImGui::BeginTabItem("Objects")) {
            drawOutlinerObjects();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Materials")) {
            drawMaterialList();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Textures")) {
            drawTextureList();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Mediums")) {
            drawMediumList();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Gui::drawOutlinerObjects() {
    static char filter[128] = "";
    ImGui::SetNextItemWidth(-ImGui::GetFrameHeight());
    ImGui::InputTextWithHint("##filter", "Filter objects...", filter, sizeof(filter));
    ImGui::SameLine();
    if(ImGui::Button("X", ImVec2(ImGui::GetFrameHeight(), 0.0f))) {
        filter[0] = '\0';
    }

    ImGui::Separator();

    const auto& objects = scene.getObjects();

    if(filter[0] == '\0') {
        ImGui::TextDisabled("%zu object(s)", objects.size());
    }

    std::string needle = toLower(filter);
    auto matches = [&needle](const std::string& name) {
        return needle.empty() || toLower(name).find(needle) != std::string::npos;
    };

    // group by concrete type so the outliner reads like a scene tree
    std::map<std::string, std::vector<std::pair<size_t, Hittable*>>> groups;
    for(size_t i = 0; i < objects.size(); i++) {
        Hittable& object = *objects[i];
        groups[getTypeName(typeid(object))].push_back({i, &object});
    }

    for(auto& [type, entries] : groups) {
        bool has_visible_entry = false;
        for(const auto& [index, object] : entries) {
            if(matches(object->getName())) {
                has_visible_entry = true;
                break;
            }
        }
        if(!has_visible_entry) continue;

        ImGui::PushID(type.c_str());
        char type_label[160];
        std::snprintf(type_label, sizeof(type_label), "%s (%zu)", type.c_str(), entries.size());

        if(ImGui::TreeNodeEx(
            type_label,
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth
        )) {
            for(const auto& [index, object] : entries) {
                if(!matches(object->getName())) continue;

                char label[160];
                std::snprintf(
                    label,
                    sizeof(label),
                    "[%zu] %s",
                    index,
                    object->getName().c_str()
                );

                if(ImGui::Selectable(label, object == selected_object)) {
                    selected_object = object;
                    requested_properties_tab = PropertiesTab::Object;
                    properties_tab_requested = true;
                }
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}

void Gui::drawPropertiesWindow() {
    ImGui::Begin("Properties");

    auto tab_flags = [this](PropertiesTab tab) {
        if(properties_tab_requested && requested_properties_tab == tab) {
            return ImGuiTabItemFlags_SetSelected;
        }
        return ImGuiTabItemFlags_None;
    };

    if(ImGui::BeginTabBar("##properties_tabs", ImGuiTabBarFlags_Reorderable)) {
        if(ImGui::BeginTabItem("Render", nullptr, tab_flags(PropertiesTab::Render))) {
            drawTracerTab();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Camera", nullptr, tab_flags(PropertiesTab::Camera))) {
            drawCameraTab();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Post-process", nullptr, tab_flags(PropertiesTab::PostProcess))) {
            drawPostprocessingTab();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Object", nullptr, tab_flags(PropertiesTab::Object))) {
            drawObjectTab();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Material", nullptr, tab_flags(PropertiesTab::Material))) {
            drawMaterialTab();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Texture", nullptr, tab_flags(PropertiesTab::Texture))) {
            drawTextureTab();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Medium", nullptr, tab_flags(PropertiesTab::Medium))) {
            drawMediumTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();

        properties_tab_requested = false;
    }

    ImGui::End();
}

void Gui::drawObjectTab() {
    drawObjectPropertiesSection(selected_object);
}

void Gui::drawMaterialList() {
    const auto& materials = scene.getMaterials();

    ImVec2 list_size(
        ImGui::GetContentRegionAvail().x,
        6.0f * ImGui::GetTextLineHeightWithSpacing()
    );
    if(ImGui::BeginListBox("##materials", list_size)) {
        for(size_t i = 0; i < materials.size(); i++) {
            Material& mat = *materials[i];
            char label[160];
            std::snprintf(label, sizeof(label), "[%zu] %s", i, mat.getName().c_str());

            if(ImGui::Selectable(label, &mat == selected_material)) {
                selected_material = &mat;
                requested_properties_tab = PropertiesTab::Material;
                properties_tab_requested = true;
            }
        }
        ImGui::EndListBox();
    }
}

void Gui::drawMaterialTab() {
    drawMaterialPropertiesSection(selected_material);
}

void Gui::drawTextureList() {
    const auto& textures = scene.getTextures();

    ImVec2 list_size(
        ImGui::GetContentRegionAvail().x,
        6.0f * ImGui::GetTextLineHeightWithSpacing()
    );
    if(ImGui::BeginListBox("##textures", list_size)) {
        for(size_t i = 0; i < textures.size(); i++) {
            Texture& tex = *textures[i];
            char label[160];
            std::snprintf(label, sizeof(label), "[%zu] %s", i, tex.getName().c_str());

            if(ImGui::Selectable(label, &tex == selected_texture)) {
                selected_texture = &tex;
                requested_properties_tab = PropertiesTab::Texture;
                properties_tab_requested = true;
            }
        }
        ImGui::EndListBox();
    }
}

void Gui::drawTextureTab() {
    drawTexturePropertiesSection(selected_texture);
}

void Gui::drawMediumList() {
    const auto& mediums = scene.getMediums();

    ImVec2 list_size(
        ImGui::GetContentRegionAvail().x,
        6.0f * ImGui::GetTextLineHeightWithSpacing()
    );
    if(ImGui::BeginListBox("##mediums", list_size)) {
        for(size_t i = 0; i < mediums.size(); i++) {
            Medium& med = *mediums[i];
            char label[160];
            std::snprintf(label, sizeof(label), "[%zu] %s", i, med.getName().c_str());

            if(ImGui::Selectable(label, &med == selected_medium)) {
                selected_medium = &med;
                requested_properties_tab = PropertiesTab::Medium;
                properties_tab_requested = true;
            }
        }
        ImGui::EndListBox();
    }
}

void Gui::drawMediumTab() {
    drawMediumPropertiesSection(selected_medium);
}

void Gui::drawCameraTab() {
    float focal_length = camera.getFocalLength();
    if(ImGui::DragFloat("Focal length", &focal_length, 0.01f, 0.01f, 100.0f)) {
        postAction([this, focal_length]() {
            camera.setFocalLength(focal_length);
        });
    }

    float aperture = camera.getAperture();
    if(ImGui::DragFloat("Aperture", &aperture, 0.01f, 0.00f, 1000.0f)) {
        postAction([this, aperture]() {
            camera.setAperture(aperture);
        });
    }

    Vec3 position = camera.getPosition();
    if(ImGui::DragFloat3("Position", &position[0], 0.05f)) {
        postAction([this, position]() {
            camera.setPosition(position);
        });
    }

    Vec3 lookat = camera.getDirection();
    if(ImGui::DragFloat3("Look at", &lookat[0], 0.05f)) {
        postAction([this, lookat]() {
            camera.lookAt(lookat);
        });
    }
}

void Gui::drawPostprocessingTab() {
    if(ImGui::Combo(
        "Tonemap",
        &(int&)tonemap,
        "Reinhard\0Exposure\0"
    )) {
        is_display_dirty = true;
    }

    if(ImGui::DragFloat("Exposure", &exposure, 0.05f, 0.01f, 100.0f)) {
        is_display_dirty = true;
    }

    if(ImGui::DragFloat("Gamma", &gamma, 0.01f, 0.1f, 5.0f)) {
        is_display_dirty = true;
    }
}

void Gui::drawTracerTab() {
    if(ImGui::Button(is_rendering_paused ? "Resume" : "Pause")) {
        if(is_rendering_paused) {
            // resuming past a reached limit turns the limit off; leaving
            // it on would pause again as soon as the next pass finishes
            if(frame_limit_reached) {
                limit_frame = false;
            }
            manual_pause = false;
            frame_limit_reached = false;
        } else {
            manual_pause = true;
        }
    }

    ImGui::SameLine();

    // queued like any other edit, so it only runs once the current
    // pass has finished and released the scene mutex
    if(ImGui::Button("Reset")) {
        postAction([this]() {
            tracer.reset();
            // a reset drops the frame limit along with the accumulated
            // result, and clears the published frame so the reset is
            // visible even while the renderer is paused
            frame_limit_reached = false;
            clearDisplayedFrame();
        });
    }

    ImGui::SameLine();

    if(ImGui::Button("Save")) {
        saveWithDialog();
    }

    ImGui::Separator();

    unsigned pass = tracer.getPass();
    if(pass > 0) {
        ImGui::Text("frame #%d, dt %d ms", pass - 1, last_dt);
        ImGui::Text("total: %dms", total_time);
        ImGui::Text("mean dt: %.3fms", (float)total_time / pass);
    } else {
        ImGui::Text("frame #0, dt n/a");
        ImGui::Text("total: n/a");
        ImGui::Text("mean dt: n/a");
    }

    if(manual_pause) {
        ImGui::TextDisabled("Renderer paused manually.");
    } else if(frame_limit_reached) {
        ImGui::TextDisabled("Renderer paused: frame limit reached.");
    }

    bool limit = limit_frame.load();
    if(ImGui::Checkbox("Limit frame count", &limit)) {
        limit_frame = limit;
        // the limit no longer applies, so release any pause it caused
        frame_limit_reached = false;
    }

    if(limit) {
        int tf = (int)target_frame.load();
        if(ImGui::InputInt("Target frame", &tf, 10, 100)) {
            if(tf < 0) tf = 0;
            target_frame = (unsigned)tf;
            // a new target allows rendering to continue
            frame_limit_reached = false;
        }
    }

    ImGui::Separator();

    int viewport_width = tracer.getViewportWidth();
    if(ImGui::DragInt("Viewport width", &viewport_width, 10, 10, 1920)) {
        postAction([this, viewport_width]() {
            tracer.setViewportWidth(viewport_width);
            sf::Vector2u new_tex_size = sf::Vector2u(
                tracer.getViewportWidth(),
                tracer.getViewportHeight()
            );
            if(texture.resize(new_tex_size)) {
                is_data_ready = false;
                tracer_buffer_copy.resize(new_tex_size.x * new_tex_size.y);
                display_pixels.resize(new_tex_size.x * new_tex_size.y);
            }
        });
    }

    int viewport_height = tracer.getViewportHeight();
    if(ImGui::DragInt("Viewport height", &viewport_height, 10, 10, 1080)) {
        postAction([this, viewport_height]() {
            tracer.setViewportHeight(viewport_height);
            sf::Vector2u new_tex_size = sf::Vector2u(
                tracer.getViewportWidth(),
                tracer.getViewportHeight()
            );
            if(texture.resize(new_tex_size)) {
                is_data_ready = false;
                tracer_buffer_copy.resize(new_tex_size.x * new_tex_size.y);
                display_pixels.resize(new_tex_size.x * new_tex_size.y);
            }
        });
    }

    int max_bounces = tracer.getMaxBounces();
    if(ImGui::DragInt("Max bounces", &max_bounces, 1, 1, 100)) {
        postAction([this, max_bounces]() {
            tracer.setMaxBounces(max_bounces);
        });
    }

    int samples_per_pixel = tracer.getSamplesPerPixel();
    if(ImGui::DragInt("Samples per pixel", &samples_per_pixel, 1, 1, 128)) {
        postAction([this, samples_per_pixel]() {
            tracer.setSamplesPerPixel(samples_per_pixel);
        });
    }
}

void Gui::drawObjectPropertiesSection(Hittable* object) {
    if(object == nullptr) {
        ImGui::TextDisabled("No object selected.");
        return;
    }

    ImGui::Text("Name: %s", object->getName().c_str());
    ImGui::Text("Type: %s", getTypeName(typeid(*object)).c_str());

    // NOTE:
    // after posting edits, the GUI still takes the stale value
    // of the props to display if the edits still havent been applied
    // which may causes some visual bugs
    // it is kinda hard to to fix, and the bug is minor anyways

    // FIXME: add object ptr to the change queue merging logics

    Transform edited = object->getTransform();
    ImGui::SeparatorText("Transform");

    bool changed = false;

    Vec3 e_translation = edited.getTranslation();
    changed |= ImGui::DragFloat3("Translation", &e_translation.x, 0.05f);
    edited.move(e_translation);

    Vec3 e_scale = edited.getSize();
    ImGui::Checkbox("Uniform scale", &uniform_scale);
    if(uniform_scale) {
        float s = (e_scale.x + e_scale.y + e_scale.z) / 3.0f;
        if(ImGui::DragFloat("Scale", &s, 0.01f)) {
            edited.scale(s);
            changed = true;
        }
    } else {
        changed |= ImGui::DragFloat3("Scale", &e_scale.x, 0.01f);
        edited.scale(e_scale);
    }

    Vec3 e_angles = edited.getRotation();
    if(ImGui::DragFloat3("Rotation (rad)", &e_angles[0], 0.01f)) {
        edited.rotate(e_angles);
        changed = true;
    }

    if(changed) {
        postAction([this, obj = object, edited]() {
            obj->setTransform(edited);
            is_geometry_dirty = true;
        });
    }

    if(Sphere* sphere = dynamic_cast<Sphere*>(object)) {
        float radius = sphere->getRadius();
        if(ImGui::DragFloat("Radius", &radius, 0.01f, 0.001f, 1000.0f)) {
            postAction([this, sphere, radius]() {
                sphere->setRadius(radius);
                is_geometry_dirty = true;
            });
        }
    }
}

void Gui::drawMaterialPropertiesSection(Material* material) {
    if(material == nullptr) {
        ImGui::TextDisabled("No material selected.");
        return;
    }

    ImGui::Text("Name: %s", material->getName().c_str());
    ImGui::Text("Type: %s", getTypeName(typeid(*material)).c_str());

    if(Metal* m = dynamic_cast<Metal*>(material)) {
        float roughness = m->getRoughness();
        if(ImGui::DragFloat("Roughness", &roughness, 0.01f, 0.0f, 1.0f)) {
            postAction([m, roughness]() {
                m->getRoughness() = roughness;
            });
        }
    } else if(Dielectric* d = dynamic_cast<Dielectric*>(material)) {
        // nothing
    } else if(dynamic_cast<Matte*>(material)) {
        // nothing
    } else if(Light* l = dynamic_cast<Light*>(material)) {
        float strength = l->getEmissionStrength();
        if(ImGui::DragFloat("Emission strength", &strength, 0.1f, 0.0f, 1000.0f)) {
            postAction([l, strength]() {
                l->getEmissionStrength() = strength;
            });
        }
    } else {
        ImGui::Text("not implemented");
    }
}

void Gui::drawTexturePropertiesSection(Texture* texture) {
    if(texture == nullptr) {
        ImGui::TextDisabled("No texture selected.");
        return;
    }

    ImGui::Text("Name: %s", texture->getName().c_str());
    ImGui::Text("Type: %s", getTypeName(typeid(*texture)).c_str());
    if(ColorTexture* ct = dynamic_cast<ColorTexture*>(texture)) {
        Color tc = ct->getColor();
        if(ImGui::ColorEdit3("Color", &tc[0])) {
            postAction([ct, tc]() {
                ct->setColor(tc);
            });
        }
    } else if(texture) {
        ImGui::Text("not implemented");
    }
}

void Gui::drawMediumPropertiesSection(Medium* medium) {
    if(medium == nullptr) {
        ImGui::TextDisabled("No medium selected.");
        return;
    }

    ImGui::Text("Name: %s", medium->getName().c_str());
    ImGui::Text("Type: %s", getTypeName(typeid(*medium)).c_str());

    float ior = medium->getIOR();
    if(ImGui::DragFloat("IOR", &ior, 0.01f, 0.01f, 10.0f)) {
        postAction([medium, ior]() {
            medium->setIOR(ior);
        });
    }

    if(Homogeneous* h = dynamic_cast<Homogeneous*>(medium)) {
        Color sigma_a = h->getSigmaA();
        if(ImGui::ColorEdit3("Absorption (sigma_a)", &sigma_a[0])) {
            postAction([h, sigma_a]() {
                h->setSigmaA(sigma_a);
            });
        }

        Color sigma_s = h->getSigmaS();
        if(ImGui::ColorEdit3("Scattering (sigma_s)", &sigma_s[0])) {
            postAction([h, sigma_s]() {
                h->setSigmaS(sigma_s);
            });
        }

        float density = h->getDensity();
        if(ImGui::DragFloat("Density", &density, 0.01f, 0.0f, 1000.0f)) {
            postAction([h, density]() {
                h->setDensity(density);
            });
        }
    } else {
        ImGui::Text("not implemented");
    }
}

void Gui::run() {
    is_gui_running = true;
    render_thread = std::thread(&Gui::renderRoutine, this);

    sf::Clock delta_clock;

    while(window.isOpen()) {
        // process events
        while(const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if(event->is<sf::Event::Closed>()) {
                window.close();
                break;
            }
        }

        if(!window.isOpen()) {
            break;
        }

        if(is_display_dirty || is_data_ready) {
            // apply post processing to the copied buffer
            // whenever data is ready or post processing params changed
            resolvePixels(
                tracer_buffer_copy,
                display_pixels,
                front_pass,
                exposure,
                gamma,
                tonemap
            );
            is_display_dirty = false;

            // then change the displaying image
            texture.update(
                reinterpret_cast<const std::uint8_t*>(
                    display_pixels.data()
                )
            );
            is_data_ready = false;
        }

        sf::Time time_elapsed = delta_clock.restart();
        ImGui::SFML::Update(window, time_elapsed);

        drawMainMenuBar();
        drawDockspace();

        drawRenderWindow();
        drawOutlinerWindow();
        drawPropertiesWindow();

        // picking uses the image rectangle recorded while drawing the
        // render window, so it must run after the windows are submitted
        handlePicking();

        // apply whatever the widgets changed this frame
        // try_lock only, never blocking
        applyPendingActions();

        // the renderer must never stay paused just because a tab is not
        // visible: the pause state is derived every frame instead
        updateRenderState();

        window.clear(sf::Color(24, 24, 24));
        ImGui::SFML::Render(window);
        window.display();
    }

    is_gui_running = false;
    render_thread.join();
}
