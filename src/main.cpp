#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>

#include <atomic>
#include <memory>
#include <thread>

#include "RayTracer.h"
#include "rendering/texture/ColorTexture.h"
#include "rendering/material/Metal.h"
#include "rendering/hittable/Sphere.h"

#include "imgui.h"
#include "imgui-SFML.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

std::atomic<bool> is_running{false};
std::atomic<bool> is_data_ready{false};

unsigned pass = 0;

void render_routine(RayTracer* tracer) {
    while(!is_running) std::this_thread::yield();

    while(is_running) {
        if (!is_data_ready) {
            tracer->render(pass++);
            is_data_ready = true;
        } else {
            std::this_thread::yield(); 
        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "tdt");
    if(!ImGui::SFML::Init(window)) {
        throw std::runtime_error("Failed to initialize imgui.");
    }

    window.setFramerateLimit(60);

    Camera camera(
        1.0f,
        Vec3(0, 0, 0),
        Vec3(0, 0, -1)
    );
    Scene scene;
    sf::Vector2u window_size = window.getSize();
    RayTracer rt(window_size.x, window_size.y, camera, scene);

    Texture& tex = scene.add_texture(std::make_unique<ColorTexture>(Color(1, 1, 1)));
    Material& mat = scene.add_material(std::make_unique<Metal>(&tex, 1.0));
    scene.add_object(std::make_unique<Sphere>(&mat, Vec3(1, 0, -6), 1.0f));

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
