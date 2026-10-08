#include "gui/Gui.h"
#include "rendering/Camera.h"
#include "rendering/Scene.h"
#include "rendering/texture/ColorTexture.h"
#include "rendering/material/Metal.h"
#include "rendering/material/Matte.h"
#include "rendering/material/Dielectric.h"
#include "rendering/material/Light.h"
#include "rendering/hittable/Mesh.h"
#include "rendering/hittable/Triangle.h"
#include "rendering/hittable/Sphere.h"

int main() {
    Camera camera(1.0f, Vec3(-3, 5, 10), Vec3(0, 0, 0));
    Scene scene;

    Texture& gray_tex = scene.addTexture(
        std::make_unique<ColorTexture>(Color(0.98))
    );
    Material& gray_matte_mat = scene.addMaterial(
        std::make_unique<Matte>(&gray_tex)
    );

    Texture& gold_tex = scene.addTexture(
        std::make_unique<ColorTexture>(Color(1, 0.894f, 0.29f))
    );
    Material& gold_metal_mat = scene.addMaterial(
        std::make_unique<Metal>(&gold_tex, 0.2f)
    );

    Texture& white_tex = scene.addTexture(
        std::make_unique<ColorTexture>(Color(1))
    );
    Material& white_dielec_mat = scene.addMaterial(
        std::make_unique<Dielectric>(&white_tex, 1.52f)
    );

    Texture& light_tex = scene.addTexture(
        std::make_unique<ColorTexture>(Color(1))
    );
    Material& light_mat = scene.addMaterial(
        std::make_unique<Light>(&light_tex, 32.0f, Color(1))
    );

    scene.addObject(std::make_unique<Triangle>(
        "platform",
        &gray_matte_mat,
        Vec3(0, 0, -100),
        Vec3(100, 0, 10),
        Vec3(-100, 0, 10)
    ));

    Hittable& teapot = scene.addObject(std::make_unique<Mesh>(
        "teapot",
        &gold_metal_mat,
        "assets/model/teapot.obj"
    ));
    teapot.getTransform().rotate(Vec3(0, 0, 0)).move(Vec3(0, 0, 0));

    scene.addObject(std::make_unique<Sphere>(
        "dielec",
        &white_dielec_mat,
        Vec3(1.2, 1, 4.4),
        1
    ));

    scene.addObject(std::make_unique<Sphere>(
        "light",
        &light_mat,
        Vec3(1.2, 5, 4.4),
        2
    ));

    Hittable& dodecahedron = scene.addObject(std::make_unique<Mesh>(
        "dodecahedron",
        &white_dielec_mat,
        "assets/model/dodecahedron.obj"
    ));
    dodecahedron.getTransform().rotate(Vec3(0, 0, 0)).move(Vec3(-3, 1.5, 2.0));

    scene.buildBVH();

    Gui gui(camera, scene, 400, 300);
    gui.run();
}
