#include <SFML/Graphics.hpp>
#include <thread>

#include "RayTracer.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

void drawingThread(sf::RenderWindow* window, sf::Sprite* sprite) {
    if(!window->setActive(true)) {
        throw std::runtime_error("Failed to activate window context on draw thread.");
    }

    while(window->isOpen()) {
        window->draw(*sprite);
        window->display();
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "OpenGL");
    if(!window.setActive(false)) {
        throw std::runtime_error("Failed to deactivate window context on main thread.");
    }

    sf::Vector2u window_size = window.getSize();

    RayTracer rt(window_size.x, window_size.y);

    sf::Texture texture(window_size);
    sf::Sprite sprite(texture);

    std::thread draw_thread(&drawingThread, &window, &sprite);

    while(window.isOpen()) {
        while(const std::optional event = window.pollEvent()) {
            if(event->is<sf::Event::Closed>())
                window.close();
        }

        rt.render();
        texture.update(rt.getData());
    }

    draw_thread.join();
}
