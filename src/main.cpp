#include "gui/Gui.h"
#include "rendering/Camera.h"
#include "rendering/Scene.h"
#include "rendering/Medium.h"
#include "rendering/texture/ColorTexture.h"
#include "rendering/material/Matte.h"
#include "rendering/material/Light.h"
#include "rendering/hittable/Mesh.h"

#include <memory>
#include <string>
#include <numbers>

int main() {
    const std::string plane_model = "assets/model/plane.obj";
    const std::string cube_model = "assets/model/cube.obj";
    const float pi = std::numbers::pi_v<float>;

    Camera camera(2.0f, Vec3(0, 1, 2.5f), Vec3(0, 1, 0));

    Scene scene;

    Medium& vacuum = scene.addMedium(std::make_unique<Medium>("vacuum"));
    scene.getGlobalMedium() = &vacuum;

    Texture& white_tex = scene.addTexture(
        std::make_unique<ColorTexture>("white tex", Color(0.73f))
    );
    Texture& red_tex = scene.addTexture(
        std::make_unique<ColorTexture>("red tex", Color(0.63f, 0.065f, 0.05f))
    );
    Texture& green_tex = scene.addTexture(
        std::make_unique<ColorTexture>("green tex", Color(0.14f, 0.45f, 0.091f))
    );
    Texture& light_tex = scene.addTexture(
        std::make_unique<ColorTexture>("light tex", Color(1))
    );

    Material& white_matte = scene.addMaterial(
        std::make_unique<Matte>("white matte", &white_tex)
    );
    Material& red_matte = scene.addMaterial(
        std::make_unique<Matte>("red matte", &red_tex)
    );
    Material& green_matte = scene.addMaterial(
        std::make_unique<Matte>("green matte", &green_tex)
    );
    Material& light_mat = scene.addMaterial(
        std::make_unique<Light>("area light", &light_tex, 15.0f, Color(1))
    );

    auto addMesh = [&scene](
        const std::string& name,
        Material& material,
        const std::string& model
    ) -> Mesh& {
        return static_cast<Mesh&>(scene.addObject(
            std::make_unique<Mesh>(name, &material, model)
        ));
    };

    auto addPlane = [&](
        const std::string& name,
        Material& material,
        const Vec3& position,
        const Vec3& rotation = Vec3(0),
        const Vec3& scale = Vec3(1)
    ) {
        Mesh& plane = addMesh(name, material, plane_model);
        plane.getTransform().size(scale).rotate(rotation).move(position);
    };

    addPlane("floor", white_matte, Vec3(0, 0, 0));
    addPlane("ceiling", white_matte, Vec3(0, 2, 0), Vec3(pi, 0, 0));
    addPlane("back wall", white_matte, Vec3(0, 1, -1), Vec3(pi / 2, 0, 0));
    addPlane("red wall", red_matte, Vec3(1, 1, 0), Vec3(0, 0, pi / 2));
    addPlane("green wall", green_matte, Vec3(-1, 1, 0), Vec3(0, 0, -pi / 2));

    addPlane(
        "light",
        light_mat,
        Vec3(0, 1.99f, 0),
        Vec3(pi, 0, 0),
        Vec3(0.3f, 1, 0.2f)
    );

    Mesh& tall_box = addMesh("tall box", white_matte, cube_model);
    tall_box.getTransform()
        .size(Vec3(0.25f, 0.6f, 0.25f))
        .rotate(Vec3(0, 0.30f, 0))
        .move(Vec3(0.42f, 0.6f, -0.35f));

    Mesh& short_box = addMesh("short box", white_matte, cube_model);
    short_box.getTransform()
        .size(Vec3(0.3f))
        .rotate(Vec3(0, -0.40f, 0))
        .move(Vec3(-0.45f, 0.3f, 0.15f));

    scene.buildBVH();

    Gui gui(camera, scene, 800, 600);
    gui.run();
}
