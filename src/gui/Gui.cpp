#include "Gui.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cstring>

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

void Gui::postNonResetAction(std::function<void()> action) {
    pending_non_reset_actions.push_back(std::move(action));
    lock_requested = true;
}

void Gui::applyPendingActions() {
    if(pending_actions.empty() and pending_non_reset_actions.empty()) return;

    std::unique_lock<std::mutex> lock(scene_mutex, std::try_to_lock);

    // only apply changes when the tracer is done rendered
    // i.e the scene is not locked
    if(!lock.owns_lock()) return;

    // apply non reset actions
    for(const auto& edit : pending_non_reset_actions) {
        edit();
    }
    pending_non_reset_actions.clear();

    if(not pending_actions.empty()) {
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
        total_time = 0;
        last_dt = 0;
    }

    lock_requested = false;
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

// selecting hierachy:
// object - material - texture
//                   \ medium
// selecting a node will cascade selecting childs (default to nullptr)
// selecting a node with the open_menu option will cascading reset parents and siblings

void Gui::setSelectedObject(const Hittable* object, bool open_menu) {
    selected_object = nullptr;
    selected_material = nullptr;
    selected_texture = nullptr;
    selected_medium = nullptr;

    if(!object)
        return;

    selected_object = const_cast<Hittable*>(object);
    if(const Material* mat = selected_object->getMaterial()) {
        setSelectedMaterial(mat, false);
    }

    if(open_menu) {
        requested_properties_tab = PropertiesTab::Object;
        properties_tab_requested = true;
    }
}

void Gui::setSelectedMaterial(const Material* material, bool open_menu) {
    selected_material = nullptr;
    selected_texture = nullptr;
    selected_medium = nullptr;

    if(!material)
        return;

    selected_material = const_cast<Material*>(material);

    if(const Texture* tex = material->getTexture())
        setSelectedTexture(tex, false);

    if(const Medium* med = material->getMedium())
        setSelectedMedium(med, false);

    if(open_menu) {
        selected_object = nullptr;
        requested_properties_tab = PropertiesTab::Material;
        properties_tab_requested = true;
    }
}

void Gui::setSelectedTexture(const Texture* texture, bool open_menu) {
    selected_texture = nullptr;

    if(!texture)
        return;

    selected_texture = const_cast<Texture*>(texture);

    if(open_menu) {
        selected_object = nullptr;
        selected_material = nullptr;
        selected_medium = nullptr;
        requested_properties_tab = PropertiesTab::Texture;
        properties_tab_requested = true;
    }
}

void Gui::setSelectedMedium(const Medium* medium, bool open_menu) {
    selected_medium = nullptr;

    if(!medium)
        return;

    selected_medium = const_cast<Medium*>(medium);

    if(open_menu) {
        selected_object = nullptr;
        selected_material = nullptr;
        selected_texture = nullptr;
        requested_properties_tab = PropertiesTab::Medium;
        properties_tab_requested = true;
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
    Hittable* obj = nullptr;
    HitInfo hit = scene.getClosest(ray, obj);
    setSelectedObject(obj);
}

void Gui::drawMainMenuBar() {
    if(!ImGui::BeginMainMenuBar()) return;

    if(ImGui::BeginMenu("File")) {
        if(ImGui::MenuItem("Save render")) {
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

    static bool layout_initialized = false;
    if(!layout_initialized) {
        layout_initialized = true;
        reset_layout_requested = true;
    }

    if(reset_layout_requested) {
        reset_layout_requested = false;

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

        ImGuiID render_node = dockspace_id;
        ImGuiID side_node = ImGui::DockBuilderSplitNode(
            render_node,
            ImGuiDir_Right,
            0.28f,
            nullptr,
            &render_node
        );

        ImGuiID listing_node = side_node;
        ImGuiID properties_node = ImGui::DockBuilderSplitNode(
            side_node,
            ImGuiDir_Down,
            0.68f,
            nullptr,
            &listing_node
        );

        ImGui::DockBuilderDockWindow("Render", render_node);
        ImGui::DockBuilderDockWindow("Listing", listing_node);
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

void Gui::drawListingWindow() {
    ImGui::Begin("Listing");

    if(ImGui::BeginTabBar("##listing_tabs", ImGuiTabBarFlags_Reorderable)) {
        if(ImGui::BeginTabItem("Objects")) {
            drawListing("object", scene.getObjects(), selected_object,
                [this](const Hittable* object, bool open_menu) { setSelectedObject(object, open_menu); });
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Materials")) {
            drawListing("material", scene.getMaterials(), selected_material,
                [this](const Material* material, bool open_menu) { setSelectedMaterial(material, open_menu); });
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Textures")) {
            drawListing("texture", scene.getTextures(), selected_texture,
                [this](const Texture* texture, bool open_menu) { setSelectedTexture(texture, open_menu); });
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Mediums")) {
            drawListing("medium", scene.getMediums(), selected_medium,
                [this](const Medium* medium, bool open_menu) { setSelectedMedium(medium, open_menu); });
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
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
            drawObjectProperties(selected_object);
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Material", nullptr, tab_flags(PropertiesTab::Material))) {
            drawMaterialProperties(selected_material);
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Texture", nullptr, tab_flags(PropertiesTab::Texture))) {
            drawTextureProperties(selected_texture);
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Medium", nullptr, tab_flags(PropertiesTab::Medium))) {
            drawMediumProperties(selected_medium);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();

        properties_tab_requested = false;
    }

    ImGui::End();
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
            // just post an empty action and the gui loop will do the rest
        });
    }

    ImGui::SameLine();

    if(ImGui::Button("Save")) {
        saveWithDialog();
    }

    ImGui::Separator();

    unsigned pass = tracer.getPass();
    if(pass > 0) {
        ImGui::Text("frame #%d, dt %d ms", pass, last_dt);
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

    bool lazy = tracer.getLazy();
    if(ImGui::Checkbox("Lazy rendering", &lazy)) {
        postNonResetAction([this, lazy]() {
            tracer.setLazy(lazy);
        });
    }

    bool limit = limit_frame.load();
    if(ImGui::Checkbox("Limit frame count", &limit)) {
        limit_frame = limit;
        frame_limit_reached = false;
    }

    if(limit) {
        int tf = (int)target_frame.load();
        if(ImGui::InputInt("Target frame", &tf, 10, 100)) {
            if(tf < 0) tf = 0;
            target_frame = (unsigned)tf;
            frame_limit_reached = false;
        }
    }

    ImGui::Separator();

    int viewport_width = tracer.getViewportWidth();
    if(ImGui::InputInt("Viewport width", &viewport_width, 10, 100)) {
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
    if(ImGui::InputInt("Viewport height", &viewport_height, 10, 100)) {
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
    if(ImGui::InputInt("Max bounces", &max_bounces, 1, 10)) {
        postNonResetAction([this, max_bounces]() {
            tracer.setMaxBounces(max_bounces);
        });
    }

    int samples_per_pixel = tracer.getSamplesPerPixel();
    if(ImGui::InputInt("Samples per pixel", &samples_per_pixel, 1, 4)) {
        postNonResetAction([this, samples_per_pixel]() {
            tracer.setSamplesPerPixel(samples_per_pixel);
        });
    }
}

void Gui::drawObjectProperties(Hittable* object) {
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

    Transform edited = object->getTransform();
    ImGui::SeparatorText("Transform");

    bool transform_changed = false;

    Vec3 e_translation = edited.getTranslation();
    if(ImGui::DragFloat3("Translation", &e_translation.x, 0.05f)) {
        edited.move(e_translation);
        transform_changed = true;
    }

    Vec3 e_angles = edited.getRotation();
    if(ImGui::DragFloat3("Rotation (rad)", &e_angles[0], 0.01f)) {
        edited.rotate(e_angles);
        transform_changed = true;
    }

    static bool uniform_scale = true;
    Vec3 e_scale = edited.getSize();
    ImGui::Checkbox("Uniform scale", &uniform_scale);
    if(uniform_scale) {
        float s = (e_scale.x + e_scale.y + e_scale.z) / 3.0f;
        if(ImGui::DragFloat("Scale", &s, 0.01f)) {
            edited.scale(s);
            transform_changed = true;
        }
    } else {
        if(ImGui::DragFloat3("Scale", &e_scale.x, 0.01f)){
            edited.scale(e_scale);
            transform_changed = true;
        }
    }

    if(transform_changed) {
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

void Gui::drawMaterialProperties(Material* material) {
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

void Gui::drawTextureProperties(Texture* texture) {
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

void Gui::drawMediumProperties(Medium* medium) {
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

void Gui::cameraControls() {
    // SFML does not have a dedicated mouse capture state
    // that stop the mouse in a specific (mostly in the middle)
    // of the screen like most game engines in wayland
    // so we have to implement our own
    // initial_mouse is used to see if the mouse is captured or not
    // if yes then it holds the position that the mouse was captured
    // else (-1, -1)

    static sf::Vector2i initial_mouse(-1, -1);
    // clock to measure delays between inputs
    static sf::Clock delta_clock;

    // only allows camera controls when holding rmb
    if(!render_view_hovered or !sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
        // release mouse
        initial_mouse.x = -1;
        initial_mouse.y = -1;
        window.setMouseCursorVisible(true);
        return;
    }

    if(initial_mouse.x == -1 and initial_mouse.y == -1) {
        // capture mouse
        initial_mouse = sf::Mouse::getPosition();
        window.setMouseCursorVisible(false);
    }

    const float SPEED = 1.0f;
    const float MOUSE_SENS = 3.34f;

    sf::Time dt = delta_clock.restart();
    float delta_sec = dt.asSeconds();

    sf::Vector2i current_mouse = sf::Mouse::getPosition();
    // mouse must be captured again after polling the position
    // giving it the biggest time window for the mouse to move a significant ammount
    sf::Mouse::setPosition(initial_mouse);

    float delta_length = SPEED * delta_sec;

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LShift))
        delta_length *= 1.7f;

    const auto& getDirection = [](sf::Keyboard::Scancode p, sf::Keyboard::Scancode n) {
        return sf::Keyboard::isKeyPressed(p) - sf::Keyboard::isKeyPressed(n);
    };

    const int trans_fwd = getDirection(
        sf::Keyboard::Scancode::W,
        sf::Keyboard::Scancode::S
    );
    const int trans_right = getDirection(
        sf::Keyboard::Scancode::D,
        sf::Keyboard::Scancode::A
    );
    const int trans_up = getDirection(
        sf::Keyboard::Scancode::E,
        sf::Keyboard::Scancode::Q
    );

    if(trans_fwd or trans_right or trans_up) {
        Vec3 translation = camera.getDirection() * trans_fwd;
        translation += camera.getRight() * trans_right;
        translation += Vec3(0, 1, 0) * trans_up;
        translation.normalize();
        translation *= delta_length;
        postAction([this, translation]() {
            camera.setPosition(camera.getPosition() + translation);
        });
    }

    sf::Vector2i delta_mouse = current_mouse - initial_mouse;
    if(delta_mouse.lengthSquared() > 0) {
        Vec3 dir = camera.getRight() * delta_mouse.x - camera.getUp() * delta_mouse.y;
        dir = dir.normalized() * MOUSE_SENS * delta_sec;
        dir = (camera.getDirection() + dir).normalized();
        if(std::abs(dir.dot(Vec3(0, 1, 0))) < 0.88f) {
            postAction([this, dir]{
                camera.setDirection(dir);
            });
        }
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
        drawListingWindow();
        drawPropertiesWindow();

        // picking uses the image rectangle recorded while drawing the
        // render window, so it must run after the windows are submitted
        handlePicking();

        // must be after handlePicking() to get the render_view_hovered flag
        cameraControls();

        // apply whatever the widgets changed this frame
        // try_lock only, never blocking
        applyPendingActions();

        updateRenderState();

        window.clear(sf::Color(24, 24, 24));
        ImGui::SFML::Render(window);
        window.display();
    }

    is_gui_running = false;
    render_thread.join();
}
