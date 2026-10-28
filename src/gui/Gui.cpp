#include "Gui.h"
#include "Material.h"
#include "Texture.h"

#include <SFML/System/Time.hpp>

#include <chrono>
#include <cstring>
#include <iostream>

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#endif

#include "imgui.h"
#include "imgui-SFML.h"

#include "rendering/texture/ColorTexture.h"
#include "rendering/material/Metal.h"
#include "rendering/material/Matte.h"
#include "rendering/material/Dielectric.h"
#include "rendering/material/Light.h"
#include "rendering/Medium.h"
#include "rendering/medium/Homogeneous.h"
#include "rendering/hittable/Mesh.h"
#include "rendering/hittable/Sphere.h"

Gui::Gui(Camera& camera, Scene& scene, unsigned width, unsigned height):
    viewport_width(width),
    viewport_height(height),
    camera(camera),
    scene(scene),
    tracer(width, height),
    window(sf::VideoMode({width, height}), "tdt"),
    texture(sf::Vector2u(width, height)),
    sprite(texture),
    front_pixels(width * height)
{
    if(!ImGui::SFML::Init(window)) {
        throw std::runtime_error("Failed to initialize imgui.");
    }

    window.setFramerateLimit(60);
    fitSprite();
}

Gui::~Gui() {
    is_running = false;
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

    if(dirty_geometry) {
        scene.buildBVH();
        dirty_geometry = false;
    }

    // the scene has changed
    // so reset the accumulated result
    tracer.reset();
    total_time = 0;
    last_dt = 0;

    lock_requested = false;
}

void Gui::fitSprite() {
    // resize the current view to fit the window size
    // to avoid texture stretching
    sf::Vector2u window_size = window.getSize();
    window.setView(sf::View(
        sf::Vector2f(window_size.x / 2.0f, window_size.y / 2.0f),
        sf::Vector2f(window_size.x, window_size.y)
    ));

    sf::Vector2u tex_size = texture.getSize();

    // the sprite caches its texture rect when the texture is assigned,
    // so it must be reset after Texture::resize or only the original
    // corner of the new texture is drawn
    sprite.setTexture(texture, true);

    float window_ratio = (float)window_size.x / window_size.y;
    float tex_ratio = (float)tex_size.x / tex_size.y;
    bool fit_width = window_ratio < tex_ratio;

    if(fit_width) {
        float scale = (float)window_size.x / tex_size.x;
        sprite.setScale(sf::Vector2f(scale, scale));
        sprite.setPosition(sf::Vector2f(0, (window_size.y - tex_size.y * scale) / 2.0f));
    } else {
        float scale = (float)window_size.y / tex_size.y;
        sprite.setScale(sf::Vector2f(scale, scale));
        sprite.setPosition(sf::Vector2f((window_size.x - tex_size.x * scale) / 2.0f, 0));
    }
}

void Gui::renderRoutine() {
    sf::Clock delta_clock;

    while(is_running) {
        // wait when is paused or the gui requested the lock
        while(is_running && (is_paused || lock_requested)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if(!is_running) break;

        // drop the time spent spinning so it is not counted as render time
        delta_clock.restart();

        {
            std::lock_guard<std::mutex> lock(scene_mutex);

            tracer.render(camera, scene);

            // publish only if the GUI consumed the previous frame, so
            // front_pixels is never read while it is being written
            if(!is_data_ready) {
                std::memcpy(
                    front_pixels.data(),
                    tracer.getData(),
                    front_pixels.size() * sizeof(std::uint32_t)
                );
                is_data_ready = true;
            }
        }

        last_dt = delta_clock.restart().asMilliseconds();
        total_time += last_dt;
        std::clog << "\r#" << tracer.getPass() << " dt: " << last_dt << "ms   " << std::flush;

        if(target_frame > 0 and tracer.getPass() > target_frame) {
            is_paused = true;
        }
    }

    if(unsigned pass = tracer.getPass())
        std::clog << "\nmean: " << (total_time / pass) << std::endl;
}

void Gui::handlePicking() {
    if(!ImGui::IsMouseClicked(ImGuiMouseButton_Left)) return;
    if(ImGui::GetIO().WantCaptureMouse) return;

    ImVec2 mouse = ImGui::GetIO().MousePos;

    sf::Vector2u window_size = window.getSize();
    if(
        mouse.x < 0
        || mouse.y < 0
        || mouse.x >= window_size.x
        || mouse.y >= window_size.y
    )
        return;

    // the sprite may be scaled to the window, so map the mouse
    // position back into texture (render buffer) pixel coordinates
    float mx = mouse.x * (float)viewport_width / window_size.x;
    float my = mouse.y * (float)viewport_height / window_size.y;

    // reproduce the renderer's viewport mapping with a single
    // camera ray shot through the clicked pixel (1-pass trace)
    float viewport_h = 2.0f;
    float viewport_w = viewport_h * ((float)viewport_width / viewport_height);

    float u = (mx - viewport_width / 2.0f) / viewport_width;
    float v = (my - viewport_height / 2.0f) / viewport_height;

    Ray ray = camera.getRayAt(Vec2(u * viewport_w, -v * viewport_h), false);
    HitInfo hit = scene.getClosest(ray, selected_object);
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
    changed |= ImGui::DragFloat3("Translation", &edited.translation.x, 0.05f);

    ImGui::Checkbox("Uniform scale", &uniform_scale);
    if(uniform_scale) {
        float s = edited.scale.x;
        if(ImGui::DragFloat("Scale", &s, 0.01f)) {
            edited.scale = Vec3(s);
            changed = true;
        }
    } else {
        changed |= ImGui::DragFloat3("Scale", &edited.scale.x, 0.01f);
    }

    Vec3 angles = edited.rotation_angles;
    if(ImGui::DragFloat3("Rotation (rad)", &angles[0], 0.01f)) {
        edited.rotate(angles);
        changed = true;
    }

    if(changed) {
        postAction([this, obj = object, edited]() {
            obj->getTransform() = edited;
            dirty_geometry = true;
        });
    }

    if(Sphere* sphere = dynamic_cast<Sphere*>(object)) {
        float radius = sphere->getRadius();
        if(ImGui::DragFloat("Radius", &radius, 0.01f, 0.001f, 1000.0f)) {
            postAction([this, sphere, radius]() {
                sphere->getRadius() = radius;
                dirty_geometry = true;
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
                ct->getColor() = tc;
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
            medium->getIOR() = ior;
        });
    }

    if(Homogeneous* h = dynamic_cast<Homogeneous*>(medium)) {
        Color sigma = h->getSigma();
        if(ImGui::ColorEdit3("Absorption", &sigma[0])) {
            postAction([h, sigma]() {
                h->getSigma() = sigma;
            });
        }

        float density = h->getDensity();
        if(ImGui::DragFloat("Density", &density, 0.01f, 0.0f, 1000.0f)) {
            postAction([h, density]() {
                h->getDensity() = density;
            });
        }
    } else {
        ImGui::Text("not implemented");
    }
}

void Gui::drawObjectsWindow() {
    ImGui::Begin("Objects");

    const auto& objects = scene.getObjects();

    for(size_t i = 0; i < objects.size(); i++) {
        Hittable& obj = *objects[i];
        char label[64];
        std::snprintf(label, sizeof(label), "[%zu] %s", i, obj.getName().c_str());

        if(ImGui::Selectable(label, &obj == selected_object)) {
            selected_object = &obj;
        }
    }

    ImGui::Separator();
    drawObjectPropertiesSection(selected_object);

    if(selected_object) {
        Material* mat = selected_object->getMaterial();
        Texture* tex = mat ? mat->getTexture() : nullptr;
        Medium* med = mat ? mat->getMedium() : nullptr;

        if(ImGui::CollapsingHeader("Material")) {
            if(mat) drawMaterialPropertiesSection(mat);
            else {
                ImGui::TextDisabled("No material assigned.");
                // add button to assign materials
            }
        } if(ImGui::CollapsingHeader("Texture")) {
            if(tex) drawTexturePropertiesSection(tex);
            else {
                ImGui::TextDisabled("No texture assigned.");
                // add button to assign textures
            }
        } if(ImGui::CollapsingHeader("Medium")) {
            if(med) drawMediumPropertiesSection(med);
            else {
                ImGui::TextDisabled("No medium assigned.");
                // add button to assign mediums
            }
        }
    }

    ImGui::End();
}

void Gui::drawMaterialsWindow() {
    ImGui::Begin("Materials");

    const auto& materials = scene.getMaterials();

    for(size_t i = 0; i < materials.size(); i++) {
        Material& mat = *materials[i];
        char label[64];
        std::snprintf(label, sizeof(label), "[%zu] %s", i, mat.getName().c_str());

        if(ImGui::Selectable(label, &mat == selected_material)) {
            selected_material = &mat;
        }
    }

    ImGui::Separator();
    drawMaterialPropertiesSection(selected_material);

    ImGui::End();
}

void Gui::drawTexturesWindow() {
    ImGui::Begin("Textures");

    const auto& textures = scene.getTextures();

    for(size_t i = 0; i < textures.size(); i++) {
        Texture& tex = *textures[i];
        char label[64];
        std::snprintf(label, sizeof(label), "[%zu] %s", i, tex.getName().c_str());

        if(ImGui::Selectable(label, &tex == selected_texture)) {
            selected_texture = &tex;
        }
    }

    ImGui::Separator();
    drawTexturePropertiesSection(selected_texture);

    ImGui::End();
}

void Gui::drawMediumsWindow() {
    ImGui::Begin("Mediums");

    const auto& mediums = scene.getMediums();

    for(size_t i = 0; i < mediums.size(); i++) {
        Medium& med = *mediums[i];
        char label[64];
        std::snprintf(label, sizeof(label), "[%zu] %s", i, med.getName().c_str());

        if(ImGui::Selectable(label, &med == selected_medium)) {
            selected_medium = &med;
        }
    }

    ImGui::Separator();
    drawMediumPropertiesSection(selected_medium);

    ImGui::End();
}

void Gui::drawCameraWindow() {
    ImGui::Begin("Camera");

    // TODO: move this to post processing window
    ImGui::DragFloat("Exposure", &camera.getExposure(), 0.05f, 0.01f, 100.0f);

    float focal_length = camera.getFocalLength();
    if(ImGui::DragFloat("Focal length", &focal_length, 0.01f, 0.01f, 100.0f)) {
        postAction([this, focal_length]() {
            camera.getFocalLength() = focal_length;
        });
    }

    float aperture = camera.getAperture();
    if(ImGui::DragFloat("Aperture", &aperture, 0.01f, 0.00f, 1000.0f)) {
        postAction([this, aperture]() {
            camera.getAperture() = aperture;
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

    ImGui::End();
}

void Gui::drawTracerWindow() {
    ImGui::Begin("Tracer");

    if(ImGui::Button(is_paused ? "Resume" : "Pause")) {
        if(target_frame == 0 or tracer.getPass() <= target_frame)
            is_paused = !is_paused;
    }

    ImGui::SameLine();

    // queued like any other edit, so it only runs once the current
    // pass has finished and released the scene mutex
    if(ImGui::Button("Reset")) {
        postAction([this]() {
            tracer.reset();
            // also clear the published frame, otherwise a reset while
            // paused keeps showing the last image
            std::memset(
                front_pixels.data(),
                0,
                front_pixels.size() * sizeof(std::uint32_t)
            );
            is_data_ready = true;
        });
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

    static bool limit_frame = true;
    ImGui::Checkbox("Limit frame count", &limit_frame);
    if(limit_frame) {
        static int tf = target_frame;
        ImGui::InputInt("Target frame", &tf, 10, 100);
        if(tf < 0) tf = 0;
        is_paused = tf <= tracer.getPass();
        target_frame = tf;
    } else {
        target_frame = 0;
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
                front_pixels.resize(new_tex_size.x * new_tex_size.y);
                fitSprite();
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
                front_pixels.resize(new_tex_size.x * new_tex_size.y);
                fitSprite();
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

    ImGui::End();
}

void Gui::run() {
    is_running = true;
    render_thread = std::thread(&Gui::renderRoutine, this);

    sf::Clock delta_clock;

    while(window.isOpen()) {
        // process events
        while(const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if(event->is<sf::Event::Closed>()) {
                window.close();
                break;
            } else if (event->is<sf::Event::Resized>()){
                fitSprite();
            }
        }

        if(!window.isOpen()) {
            break;
        }

        if(is_data_ready) {
            // front_pixels is only written while is_data_ready is false,
            // so this read never races the renderer; is_data_ready is
            // lowered only after the upload finished
            texture.update(reinterpret_cast<const std::uint8_t*>(front_pixels.data()));
            is_data_ready = false;
        }

        sf::Time time_elapsed = delta_clock.restart();
        ImGui::SFML::Update(window, time_elapsed);

        handlePicking();

        drawObjectsWindow();
        drawMaterialsWindow();
        drawTexturesWindow();
        drawMediumsWindow();
        drawCameraWindow();
        drawTracerWindow();

        // apply whatever the widgets changed this frame
        // try_lock only, never blocking
        applyPendingActions();

        window.clear();
        window.draw(sprite);
        ImGui::SFML::Render(window);
        window.display();
    }

    is_running = false;
    render_thread.join();
}
