#include "Gui.h"

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
#include "rendering/hittable/Triangle.h"
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

    // size the rendered sprite to the real window size
    sf::Vector2u window_size = window.getSize();
    sf::Vector2u tex_size = texture.getSize();
    if(tex_size.x > 0 && tex_size.y > 0) {
        sprite.setScale(sf::Vector2f(
            (float)window_size.x / tex_size.x,
            (float)window_size.y / tex_size.y
        ));
    }
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

void Gui::postEdit(EditType field, std::function<void()> edit) {
    pending_edits[field] = std::move(edit);
    lock_requested = true;
}

void Gui::applyPendingEdits() {
    if(pending_edits.empty()) return;

    std::unique_lock<std::mutex> lock(scene_mutex, std::try_to_lock);

    // only apply changes when the tracer is done rendered
    // i.e the scene is not locked
    if(!lock.owns_lock()) return;

    for(const auto& [field, edit] : pending_edits) {
        edit();
    }
    pending_edits.clear();

    if(dirty_geometry) {
        scene.buildBVH();
        dirty_geometry = false;
    }

    // the scene has changed
    // so reset the accumulated result
    tracer.reset();
    pass = 0;

    lock_requested = false;
}

void Gui::renderRoutine() {
    float total_time = 0;
    sf::Clock delta_clock;

    while(is_running) {
        // edits are queued for the scene: hold off re-acquiring the
        // lock so the GUI's try_lock() gets its chance
        while(lock_requested && is_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if(!is_running) break;

        delta_clock.restart();

        {
            std::lock_guard<std::mutex> lock(scene_mutex);

            tracer.render(camera, scene, pass);
            pass++;

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

        float dt = delta_clock.restart().asMilliseconds();
        total_time += dt;
        std::clog << "\r#" << pass << " dt: " << dt << "ms   " << std::flush;
    }

    if(pass > 0)
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
    ) {
        return;
    }

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

    ImGui::End();
}

void Gui::drawObjectPropertiesWindow() {
    ImGui::Begin("Selected Object");

    if(selected_object == nullptr) {
        ImGui::TextDisabled("No object selected.");
        ImGui::End();
        return;
    }

    ImGui::Text("Name: %s", selected_object->getName().c_str());
    ImGui::Text("Type: %s", getTypeName(typeid(*selected_object)).c_str());

    // NOTE:
    // after posting edits, the GUI still takes the stale value
    // of the props to display if the edits still havent been applied
    // which may causes some visual bugs
    // it is kinda hard to to fix, and the bug is minor anyways

    Transform edited = selected_object->getTransform();
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
        postEdit(EditType::ObjectTransform, [this, obj = selected_object, edited]() {
            obj->getTransform() = edited;
            dirty_geometry = true;
        });
    }

    if(Sphere* sphere = dynamic_cast<Sphere*>(selected_object)) {
        float radius = sphere->getRadius();
        if(ImGui::DragFloat("Radius", &radius, 0.01f, 0.001f, 1000.0f)) {
            postEdit(EditType::ObjectRadius, [this, sphere, radius]() {
                sphere->getRadius() = radius;
                dirty_geometry = true;
            });
        }
    }

    Material* mat = selected_object->getMaterial();
    if(mat != nullptr) {
        ImGui::SeparatorText("Material");
        ImGui::Text("ID: %p", mat);

        if(Metal* m = dynamic_cast<Metal*>(mat)) {
            ImGui::Text("Type: Metal");
            float roughness = m->getRoughness();
            if(ImGui::DragFloat("Roughness", &roughness, 0.01f, 0.0f, 1.0f)) {
                postEdit(EditType::MaterialRoughness, [m, roughness]() {
                    m->getRoughness() = roughness;
                });
            }
        } else if(Dielectric* d = dynamic_cast<Dielectric*>(mat)) {
            ImGui::Text("Type: Dielectric");
        } else if(dynamic_cast<Matte*>(mat)) {
            ImGui::Text("Type: Matte");
        } else if(Light* l = dynamic_cast<Light*>(mat)) {
            ImGui::Text("Type: Light");
            float strength = l->getEmissionStrength();
            if(ImGui::DragFloat("Emission strength", &strength, 0.1f, 0.0f, 1000.0f)) {
                postEdit(EditType::MaterialEmission, [l, strength]() {
                    l->getEmissionStrength() = strength;
                });
            }
        } else {
            ImGui::Text("Type: Unknown");
        }
    }

    ImGui::SeparatorText("Texture");
    Texture* tex = mat ? mat->getTexture() : nullptr;
    if(tex == nullptr) {
        ImGui::TextDisabled("None");
    } else {
        ImGui::Text("ID: %p", tex);
        if(ColorTexture* ct = dynamic_cast<ColorTexture*>(tex)) {
            ImGui::Text("Type: ColorTexture");
            Color tc = ct->getColor();
            if(ImGui::ColorEdit3("Color", &tc[0])) {
                postEdit(EditType::TextureColor, [ct, tc]() {
                    ct->getColor() = tc;
                });
            }
        } else if(tex) {
            ImGui::Text("Type: Other");
        }
    }

    Medium* med = mat ? mat->getMedium() : nullptr;
    if(med != nullptr) {
        ImGui::SeparatorText("Medium");
        ImGui::Text("ID: %p", med);

        float ior = med->getIOR();
        if(ImGui::DragFloat("IOR", &ior, 0.01f, 0.01f, 10.0f)) {
            postEdit(EditType::MediumIOR, [med, ior]() {
                med->getIOR() = ior;
            });
        }

        if(Homogeneous* h = dynamic_cast<Homogeneous*>(med)) {
            ImGui::Text("Type: Homogeneous");

            Color sigma = h->getSigma();
            if(ImGui::ColorEdit3("Absorption", &sigma[0])) {
                postEdit(EditType::MediumSigma, [h, sigma]() {
                    h->getSigma() = sigma;
                });
            }

            float density = h->getDensity();
            if(ImGui::DragFloat("Density", &density, 0.01f, 0.0f, 1000.0f)) {
                postEdit(EditType::MediumDensity, [h, density]() {
                    h->getDensity() = density;
                });
            }
        } else {
            ImGui::Text("Type: Medium");
        }
    }

    ImGui::End();
}

void Gui::drawCameraWindow() {
    ImGui::Begin("Camera");

    // TODO: move this to post processing window
    ImGui::DragFloat("Exposure", &camera.getExposure(), 0.05f, 0.01f, 100.0f);

    float focal_length = camera.getFocalLength();
    if(ImGui::DragFloat("Focal length", &focal_length, 0.01f, 0.01f, 100.0f)) {
        postEdit(EditType::CameraFocalLength, [this, focal_length]() {
            camera.getFocalLength() = focal_length;
        });
    }

    float aperture = camera.getAperture();
    if(ImGui::DragFloat("Aperture", &aperture, 0.01f, 0.00f, 1000.0f)) {
        postEdit(EditType::CameraAperture, [this, aperture]() {
            camera.getAperture() = aperture;
        });
    }

    float diverge_strength = camera.getDivergeStrength();
    if(ImGui::DragFloat("Anti-Alias strength", &diverge_strength, 0.001f, 0.00f, 0.05f)) {
        postEdit(EditType::CameraDivergeStrength, [this, diverge_strength]() {
            camera.getDivergeStrength() = diverge_strength;
        });
    }

    Vec3 position = camera.getPosition();
    if(ImGui::DragFloat3("Position", &position[0], 0.05f)) {
        postEdit(EditType::CameraPosition, [this, position]() {
            camera.setPosition(position);
        });
    }

    Vec3 lookat = camera.getDirection();
    if(ImGui::DragFloat3("Look at", &lookat[0], 0.05f)) {
        postEdit(EditType::CameraLookAt, [this, lookat]() {
            camera.lookAt(lookat);
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
        drawObjectPropertiesWindow();
        drawCameraWindow();

        // apply whatever the widgets changed this frame
        // try_lock only, never blocking
        applyPendingEdits();

        window.clear();
        window.draw(sprite);
        ImGui::SFML::Render(window);
        window.display();
    }

    is_running = false;
    render_thread.join();
}
