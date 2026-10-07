#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>

#include <iostream>
#include <atomic>
#include <memory>
#include <thread>

#include "RayTracer.h"
#include "rendering/texture/ColorTexture.h"
#include "rendering/material/Metal.h"
#include "rendering/material/Matte.h"
#include "rendering/material/Dielectric.h"
#include "rendering/material/Light.h"
#include "rendering/hittable/Mesh.h"
#include "rendering/hittable/Triangle.h"
#include "rendering/hittable/Sphere.h"

#include "imgui.h"
#include "imgui-SFML.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

std::atomic<bool> is_running{false};
std::atomic<bool> is_data_ready{false};

unsigned pass = 0;

float total_time = 0;

void render_routine(RayTracer* tracer) {
    while(!is_running) std::this_thread::yield();

    sf::Clock delta_clock;
    while(is_running) {
        tracer->render(pass++);
        sf::Time time_elapsed = delta_clock.restart();
        is_data_ready = true;
        float dt = time_elapsed.asMilliseconds();
        std::clog << "\r#" << pass << " dt: " << dt << "ms   " << std::flush;
        total_time += dt;
    }

    std::clog << "\nmean: " << total_time / pass << std::endl;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "tdt");
    if(!ImGui::SFML::Init(window)) {
        throw std::runtime_error("Failed to initialize imgui.");
    }

    window.setFramerateLimit(60);

    Camera camera(
        1.0f,
        Vec3(-3, 5, 10),
        Vec3(0, 0, 0)
    );
    Scene scene;
    sf::Vector2u window_size = window.getSize();
    RayTracer rt(window_size.x, window_size.y, camera, scene);

    Texture& gray_tex = scene.add_texture(
        std::make_unique<ColorTexture>(Color(0.98))
    );
    Material& gray_matte_mat = scene.add_material(
        std::make_unique<Matte>(&gray_tex)
    );

    Texture& gold_tex = scene.add_texture(
        std::make_unique<ColorTexture>(Color(1, 0.894f, 0.29f))
    );
    Material& gold_metal_mat = scene.add_material(
        std::make_unique<Metal>(&gold_tex, 0.2f)
    );

    Texture& white_tex = scene.add_texture(
        std::make_unique<ColorTexture>(Color(1))
    );
    Material& white_dielec_mat = scene.add_material(
        std::make_unique<Dielectric>(&white_tex, 1.52f)
    );
    Material& white_light_mat = scene.add_material(
        std::make_unique<Light>(&white_tex, 32.0f, Color(1))
    );

    Hittable& platform = scene.add_object(std::make_unique<Triangle>(
        &gray_matte_mat,
        Vec3(0, 0, -100),
        Vec3(100, 0, 10),
        Vec3(-100, 0, 10)
    ));

    Hittable& teapot = scene.add_object(std::make_unique<Mesh>(
        &gold_metal_mat,
        "assets/model/teapot.obj"
    ));
    teapot.getTransform().rotate(Vec3(0, 0, 0)).move(Vec3(0, 0, 0));

    Hittable& sphere = scene.add_object(std::make_unique<Sphere>(
        &white_dielec_mat,
        Vec3(1.2, 1, 4.4),
        1
    ));

    Hittable& light_sphere = scene.add_object(std::make_unique<Sphere>(
        &white_light_mat,
        Vec3(1.2, 5, 4.4),
        2
    ));

    Hittable& dodecahedron = scene.add_object(std::make_unique<Mesh>(
        &white_dielec_mat,
        "assets/model/dodecahedron.obj"
    ));
    dodecahedron.getTransform().rotate(Vec3(M_PIf, 0, 0)).move(Vec3(-3, 1.5, 2.0));

    scene.build_bvh();

    sf::Texture texture(window_size);
    sf::Sprite sprite(texture);

    sf::Clock delta_clock;
    float total_time = 0;

    std::thread render_thread(render_routine, &rt);
    is_running = true;

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
            texture.update(rt.getData());
            is_data_ready = false;
        }

        sf::Time time_elapsed = delta_clock.restart();
        total_time += time_elapsed.asSeconds();
        ImGui::SFML::Update(window, time_elapsed);

        ImGui::ShowDemoWindow();
        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        window.clear(); 
        window.draw(sprite);
        ImGui::SFML::Render(window);
        window.display();
    }

    is_running = false;
    render_thread.join();
    ImGui::SFML::Shutdown();
}
