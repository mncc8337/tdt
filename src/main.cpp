#include <SFML/Graphics.hpp>
#include <thread>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

void drawingThread(sf::RenderWindow* window, sf::Sprite* sprite) {
    if(!window->setActive(true)) {
        throw std::runtime_error("Failed to activate window context on draw thread.");
    }

    while(window->isOpen()) {
        window->clear();
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

    sf::Texture texture(window_size);
    std::vector<std::uint32_t> pixels(window_size.x * window_size.y, 0x000000ff);
    texture.update(reinterpret_cast<std::uint8_t*>(pixels.data()));

    sf::Sprite sprite(texture);

    std::thread draw_thread(&drawingThread, &window, &sprite);

    while(window.isOpen()) {
        while(const std::optional event = window.pollEvent()) {
            if(event->is<sf::Event::Closed>())
                window.close();
        }
    }

    draw_thread.join();
}
