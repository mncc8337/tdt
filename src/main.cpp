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
#include "rendering/Medium.h"
#include "rendering/medium/Homogeneous.h"

int main() {
    Camera camera(1.0f, Vec3(-3, 5, 10), Vec3(0, 3.75f, 0));
    Scene scene;

    Medium& vacuum = scene.addMedium(std::make_unique<Medium>());
    scene.getGlobalMedium() = &vacuum;

    Medium& glass = scene.addMedium(std::make_unique<Homogeneous>(
        1.52f,
        Color(0),
        1.0f
    ));

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
        std::make_unique<Metal>(&gold_tex, 0.25f)
    );

    Texture& white_tex = scene.addTexture(
        std::make_unique<ColorTexture>(Color(1))
    );
    Material& white_dielec_mat = scene.addMaterial(
        std::make_unique<Dielectric>(&white_tex, &glass)
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

    scene.addObject(std::make_unique<Mesh>(
        "bunny",
        &gold_metal_mat,
        "assets/model/stanford-bunny.obj"
    )).getTransform().rotate(Vec3(0.0f, 0.17f, 0.0f)).move(Vec3(-1.05f, 0.0f, 0.0f));

    scene.addObject(std::make_unique<Sphere>(
        "light",
        &light_mat,
        Vec3(0),
        7.61f
    )).getTransform().move(Vec3(15.35f, 15.4f, -9.0f));

    scene.buildBVH();

    Gui gui(camera, scene, 800, 600);
    gui.run();
}
