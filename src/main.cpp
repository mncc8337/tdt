#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>

#include <atomic>
#include <memory>
#include <thread>

#include "RayTracer.h"
#include "rendering/hittable/Sphere.h"

#include "imgui.h"
#include "imgui-SFML.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

std::atomic<bool> is_running{false};
std::atomic<bool> is_data_ready{false};

void render_routine(RayTracer* tracer) {
    while(!is_running) std::this_thread::yield();

    while(is_running) {
        if (!is_data_ready) {
            tracer->render();
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

    Sphere sphere(Vec3(0, 0, -5), 1.0f);
    scene.add_object(std::make_unique<Sphere>(sphere));

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

        // Cập nhật texture nếu render thread đã làm xong 1 frame
        if(is_data_ready) {
            texture.update(rt.getData());
            is_data_ready = false; // Báo cho render thread biết để tiếp tục làm frame mới
        }

        sf::Time time_elapsed = delta_clock.restart();
        ImGui::SFML::Update(window, time_elapsed);

        // GUI
        ImGui::ShowDemoWindow();
        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        // RENDER PIPELINE CỦA SFML (Thêm window.clear)
        window.clear(); 
        window.draw(sprite);
        ImGui::SFML::Render(window);
        window.display();
    }

    is_running = false;
    render_thread.join();
    ImGui::SFML::Shutdown();
}
