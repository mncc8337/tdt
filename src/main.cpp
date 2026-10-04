#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>

#include <iostream>
#include <atomic>
#include <memory>
#include <thread>

#include "misc/RNG.h"
#include "RayTracer.h"
#include "rendering/texture/ColorTexture.h"
#include "rendering/material/Metal.h"
#include "rendering/material/Dielectric.h"
#include "rendering/hittable/Sphere.h"
#include "rendering/hittable/Triangle.h"

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
        std::clog << "#" << pass << " dt: " << dt << "ms" << '\n';
        total_time += dt;
    }

    std::clog << "mean: " << total_time / pass << std::endl;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "tdt");
    if(!ImGui::SFML::Init(window)) {
        throw std::runtime_error("Failed to initialize imgui.");
    }

    window.setFramerateLimit(60);

    Camera camera(
        1.0f,
        Vec3(0, 5, 0),
        Vec3(0, 0, -1)
    );
    Scene scene;
    sf::Vector2u window_size = window.getSize();
    RayTracer rt(window_size.x, window_size.y, camera, scene);

    Texture& white_tex = scene.add_texture(
        std::make_unique<ColorTexture>(Color(1, 1, 1))
    );
    Material& white_metal_mat = scene.add_material(
        std::make_unique<Metal>(&white_tex, 1.0)
    );
    scene.add_object(std::make_unique<Triangle>(
        &white_metal_mat,
        Vec3(-1000, 0, -1000),
        Vec3(1000, 0, -1000),
        Vec3(0, 0, 1000)
    ));

    Material& glass_mat = scene.add_material(
        std::make_unique<Dielectric>(&white_tex, 1.52)
    );

    for(int i = 0; i < 100; i++) {
        Texture& tex = scene.add_texture(
            std::make_unique<ColorTexture>(RNG::color())
        );
        Material* mat;
        if(RNG::uniform() < 0.7) {
            mat = &scene.add_material(
                std::make_unique<Metal>(&tex, RNG::uniform())
            );
        } else {
            mat = &glass_mat;
        }
        float radius = RNG::uniform(0.5, 2);

        scene.add_object(std::make_unique<Sphere>(
            mat,
            Vec3(
                RNG::uniform(-50, 50),
                radius,
                RNG::uniform(-105, -5)
            ),
            radius
        ));
    }

    sf::Texture texture(window_size);
    sf::Sprite sprite(texture);

    sf::Clock delta_clock;

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
