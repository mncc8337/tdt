#pragma once

#include <SFML/Graphics.hpp>

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "Medium.h"
#include "Texture.h"
#include "rendering/Camera.h"
#include "rendering/RayTracer.h"
#include "rendering/Scene.h"

class Gui {
private:
    unsigned viewport_width;
    unsigned viewport_height;

    Camera& camera;
    Scene& scene;
    RayTracer tracer;

    sf::RenderWindow window;
    sf::Texture texture;
    sf::Sprite sprite;

    std::thread render_thread;
    std::atomic<bool> is_running{false};
    std::atomic<bool> is_data_ready{false};

    // the GUI never waits on the scene: edits are queued and applied
    // only when try_lock() can take scene_mutex without blocking. while
    // edits are pending, lock_requested tells the render thread to hold
    // off re-acquiring the mutex for its next pass, so the queued edits
    // get their chance instead of racing it every time.
    std::atomic<bool> lock_requested{false};
    std::vector<std::function<void()>> pending_edits;
    std::mutex scene_mutex;

    bool dirty_geometry = false;

    // finished frame, published by the render thread only while
    // is_data_ready is false and read by the GUI only while it is true
    std::vector<std::uint32_t> front_pixels;

    unsigned pass = 0;

    Hittable* selected_object = nullptr;
    Material* selected_material = nullptr;
    Texture* selected_texture = nullptr;
    Medium* selected_medium = nullptr;

    bool uniform_scale = true;

    std::string getTypeName(const std::type_info& ti);

    void postEdit(std::function<void()> edit);
    void applyPendingEdits();

    void renderRoutine();
    void handlePicking();

    void drawObjectPropertiesSection(Hittable* object);
    void drawMaterialPropertiesSection(Material* material);
    void drawTexturePropertiesSection(Texture* texture);
    void drawMediumPropertiesSection(Medium* medium);

    void drawObjectsWindow();
    void drawMaterialsWindow();
    void drawTexturesWindow();
    void drawMediumsWindow();
    void drawCameraWindow();

public:
    Gui(Camera& camera, Scene& scene, unsigned width, unsigned height);
    ~Gui();

    void run();
};
