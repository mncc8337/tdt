#pragma once

#include <SFML/Graphics.hpp>

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "Medium.h"
#include "Texture.h"
#include "math/Vec3.h"
#include "misc/ColorUtils.h"
#include "rendering/Camera.h"
#include "rendering/RayTracer.h"
#include "rendering/Scene.h"

class Gui {
private:
    Camera& camera;
    Scene& scene;
    RayTracer tracer;

    sf::RenderWindow window;
    sf::Texture texture;

    std::thread render_thread;
    // gui state
    std::atomic<bool> is_gui_running{false};
    // rendering state
    std::atomic<bool> is_rendering_paused{false};
    // effective pause is derived every frame from these two sources:
    // a manual user pause, or the frame limit having been reached
    bool manual_pause = false;
    std::atomic<bool> frame_limit_reached{false};
    std::atomic<bool> limit_frame{true};
    std::atomic<unsigned> target_frame{100};
    // raise when the tracer is done rendering a single frame
    std::atomic<bool> is_data_ready{false};

    unsigned total_time = 0;
    unsigned last_dt = 0;

    // the GUI never waits on the scene: edits are queued and applied
    // only when try_lock() can take scene_mutex without blocking. while
    // edits are pending, lock_requested tells the render thread to hold
    // off re-acquiring the mutex for its next pass, so the queued edits
    // get their chance instead of racing it every time.
    std::atomic<bool> lock_requested{false};
    std::vector<std::function<void()>> pending_actions;
    std::mutex scene_mutex;

    // raise when the scene geometry is changed
    bool is_geometry_dirty = false;

    // finished HDR frame, published by the render thread only while
    // is_data_ready is false and read by the GUI only while it is true
    std::vector<Color> tracer_buffer_copy;
    unsigned front_pass = 0;

    // resolved 8-bit frame, produced on the GUI thread at display time
    std::vector<std::uint32_t> display_pixels;

    // post-processing states
    float exposure = 1.0f;
    float gamma = 2.2f;
    Tonemap tonemap = Tonemap::Exposure;

    // raise when the raw frame needs to be post process
    bool is_display_dirty = false;

    Hittable* selected_object = nullptr;
    Material* selected_material = nullptr;
    Texture* selected_texture = nullptr;
    Medium* selected_medium = nullptr;

    bool uniform_scale = true;

    // screen-space rectangle of the rendered image inside its dock window,
    // used to map mouse clicks back into render buffer pixels
    float render_view_x = 0.0f;
    float render_view_y = 0.0f;
    float render_view_width = 0.0f;
    float render_view_height = 0.0f;
    bool render_view_hovered = false;

    // docking layout is built once, or whenever the user asks for it
    bool layout_initialized = false;
    bool layout_checked_saved = false;
    bool has_saved_layout = false;
    bool reset_layout_requested = false;
    // wait for the window manager to settle the startup window size before
    // choosing the default split, otherwise the side column can end up
    // sized for the initial window instead of the final one
    sf::Vector2u last_window_size{0, 0};
    int stable_window_frames = 0;

    enum class PropertiesTab {
        Object,
        Material,
        Texture,
        Medium,
        Camera,
        Render,
        PostProcess
    };

    // set when an outliner selection should reveal the matching tab
    PropertiesTab requested_properties_tab = PropertiesTab::Object;
    bool properties_tab_requested = false;

    std::string getTypeName(const std::type_info& ti);

    void postAction(std::function<void()> action);
    void applyPendingActions();

    bool save(const std::filesystem::path &filename);
    void saveWithDialog();

    void renderRoutine();
    // derive is_rendering_paused from the manual pause and frame limit,
    // so the render thread state does not depend on any window being drawn
    void updateRenderState();
    // must be called while holding scene_mutex
    void clearDisplayedFrame();
    void handlePicking();

    void drawMainMenuBar();
    void drawDockspace();
    void drawRenderWindow();
    void drawOutlinerWindow();
    void drawOutlinerObjects();
    void drawMaterialList();
    void drawTextureList();
    void drawMediumList();
    void drawPropertiesWindow();

    void drawObjectTab();
    void drawMaterialTab();
    void drawTextureTab();
    void drawMediumTab();
    void drawCameraTab();
    void drawTracerTab();
    void drawPostprocessingTab();

    void drawObjectPropertiesSection(Hittable* object);
    void drawMaterialPropertiesSection(Material* material);
    void drawTexturePropertiesSection(Texture* texture);
    void drawMediumPropertiesSection(Medium* medium);

public:
    Gui(Camera& camera, Scene& scene, unsigned width, unsigned height);
    ~Gui();

    void run();
};
