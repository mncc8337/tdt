#include "gui/Gui.h"
#include "rendering/Camera.h"
#include "rendering/Scene.h"
#include "rendering/Medium.h"
#include "rendering/medium/Homogeneous.h"
#include "rendering/texture/SolidTexture.h"
#include "rendering/texture/ImageTexture.h"
#include "rendering/material/Matte.h"
#include "rendering/material/Metal.h"
#include "rendering/material/Light.h"
#include "rendering/material/Dielectric.h"
#include "rendering/material/FineParticle.h"
#include "rendering/hittable/Sphere.h"
#include "rendering/hittable/Mesh.h"

#include <numbers>

void cornelBox(Scene& scene, Camera& camera) {
    const std::string plane_model = "assets/model/plane.obj";
    const std::string cube_model = "assets/model/cube.obj";
    const float pi = std::numbers::pi_v<float>;

    Medium& vacuum = scene.addMedium<Medium>("vacuum");

    Medium& glass = scene.addMedium<Homogeneous>(
        "glass",
        1.33f,
        Color(0.0f),
        Color(0.0f),
        1.0f
    );

    scene.setGlobalMedium(vacuum);

    Texture& white_tex = scene.addTexture<ColorTexture>(
        "white tex",
        Color(0.73f)
    );
    Texture& red_tex = scene.addTexture<ColorTexture>(
        "red tex",
        Color(0.63f, 0.065f, 0.05f)
    );
    Texture& green_tex = scene.addTexture<ColorTexture>(
        "green tex",
        Color(0.14f, 0.45f, 0.091f)
    );
    Texture& light_tex = scene.addTexture<ColorTexture>(
        "light tex",
        Color(1.0f)
    );
    Texture& glass_tex = scene.addTexture<ColorTexture>(
        "glass tex",
        Color(1.0f)
    );
    Texture& globe_tex = scene.addTexture<ImageTexture>(
        "globe tex",
        "assets/texture/earth.jpg"
    );

    Material& white_matte = scene.addMaterial<Matte>(
        "white matte",
        &white_tex
    );
    Material& red_matte = scene.addMaterial<Matte>(
        "red matte",
        &red_tex
    );
    Material& green_matte = scene.addMaterial<Matte>(
        "green matte",
        &green_tex
    );
    Material& light_mat = scene.addMaterial<Light>(
        "area light",
        &light_tex,
        15.0f,
        Color(1)
    );
    Material& glass_mat = scene.addMaterial<Dielectric>(
        "glass mat",
        &glass_tex,
        &glass
    );
    Material& globe_mat = scene.addMaterial<Matte>(
        "globe mat",
        &globe_tex
    );

    auto addMesh = [&scene](
        const std::string& name,
        Material& material,
        const std::string& model
    ) -> Mesh& {
        return static_cast<Mesh&>(scene.addObject<Mesh>(name, &material, model));
    };

    auto addPlane = [&](
        const std::string& name,
        Material& material,
        const Vec3& position,
        const Vec3& rotation = Vec3(0),
        const Vec3& scale = Vec3(1)
    ) {
        Mesh& plane = addMesh(name, material, plane_model);
        Transform plane_t = plane.getTransform();
        plane_t.scale(scale).rotate(rotation).move(position);
        plane.setTransform(plane_t);
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

    // Mesh& tall_box = addMesh("tall box", white_matte, cube_model);
    // Transform tall_box_t = tall_box.getTransform();
    // tall_box_t.scale(Vec3(0.25f, 0.6f, 0.25f))
    //     .rotate(Vec3(0, 0.3f, 0))
    //     .move(Vec3(0.42f, 0.605f, -0.35f));
    // tall_box.setTransform(tall_box_t);

    Hittable& sphere = scene.addObject<Sphere>("globe", &globe_mat, 0.5f);
    Transform sphere_t = sphere.getTransform();
    sphere_t.move(Vec3(0.42f, 0.55f, -0.35f));
    sphere.setTransform(sphere_t);

    Mesh& short_box = addMesh("short box", glass_mat, cube_model);
    Transform short_box_t = short_box.getTransform();
    short_box_t.scale(Vec3(0.3f))
        .rotate(Vec3(0, -0.4f, 0))
        .move(Vec3(-0.45f, 0.305f, 0.15f));
    short_box.setTransform(short_box_t);
}

void demo(Scene& scene, Camera& camera) {
    const std::string plane_model = "assets/model/plane.obj";
    const std::string cube_model = "assets/model/cube.obj";
    const std::string cube_uv_model = "assets/model/cube-uv.obj";
    const std::string bunny_model = "assets/model/stanford-bunny.obj";
    const std::string teapot_model = "assets/model/teapot.obj";
    const std::string dodecahedron_model = "assets/model/dodecahedron.obj";

    const float pi = std::numbers::pi_v<float>;

    camera.setPosition(Vec3(0.0f, 1.5f, 2.5f));
    camera.lookAt(Vec3(0.0f, 0.85f, 0.0f));
    camera.setFocalLength(1.43f);

    Medium& glass = scene.addMedium<Homogeneous>(
        "clear glass",
        1.5f,
        Color(0.0f),
        Color(0.0f),
        1.0f
    );

    Homogeneous& smoke = static_cast<Homogeneous&>(scene.addMedium<Homogeneous>(
        "smoke",
        1.0f,
        Color(0.02f),
        Color(2.0f),
        1.0f
    ));

    Medium& vacuum = scene.addMedium<Medium>("vacuum");

    scene.setGlobalMedium(vacuum);

    Texture& floor_tex = scene.addTexture<ColorTexture>(
        "floor tex",
        Color(0.55f, 0.53f, 0.49f)
    );
    Texture& wall_tex = scene.addTexture<ColorTexture>(
        "wall tex",
        Color(0.10f, 0.12f, 0.16f)
    );
    Texture& white_tex = scene.addTexture<ColorTexture>(
        "white tex",
        Color(1.0f)
    );
    Texture& gold_tex = scene.addTexture<ColorTexture>(
        "gold tex",
        Color(0.95f, 0.73f, 0.30f)
    );
    Texture& copper_tex = scene.addTexture<ColorTexture>(
        "copper tex",
        Color(0.85f, 0.40f, 0.22f)
    );
    Texture& bunny_tex = scene.addTexture<ColorTexture>(
        "bunny tex",
        Color(0.82f, 0.80f, 0.74f)
    );
    Texture& glass_tex = scene.addTexture<ColorTexture>(
        "glass tex",
        Color(1.0f)
    );
    Texture& smoke_tex = scene.addTexture<ColorTexture>(
        "smoke tex",
        Color(1.0f)
    );
    Texture& earth_tex = scene.addTexture<ImageTexture>(
        "earth tex",
        "assets/texture/earth.jpg"
    );
    Texture& sun_tex = scene.addTexture<ImageTexture>(
        "sun tex",
        "assets/texture/sun.jpg"
    );
    Texture& dice_tex = scene.addTexture<ImageTexture>(
        "dice tex",
        "assets/texture/dice.png"
    );

    Material& floor_mat = scene.addMaterial<Matte>("floor mat", &floor_tex);
    Material& wall_mat = scene.addMaterial<Matte>("wall mat", &wall_tex);
    Material& bunny_mat = scene.addMaterial<Matte>("bunny mat", &bunny_tex);
    Material& earth_mat = scene.addMaterial<Matte>("earth mat", &earth_tex);
    Material& dice_mat = scene.addMaterial<Matte>("dice mat", &dice_tex);

    Material& teapot_mat = scene.addMaterial<Metal>(
        "gold metal",
        &gold_tex,
        0.12f
    );
    Material& dodecahedron_mat = scene.addMaterial<Metal>(
        "copper metal",
        &copper_tex,
        0.35f
    );

    Material& glass_mat = scene.addMaterial<Dielectric>(
        "glass mat",
        &glass_tex,
        &glass
    );
    Material& smoke_mat = scene.addMaterial<FineParticle>(
        "smoke mat",
        &smoke_tex,
        &smoke
    );

    Material& area_light_mat = scene.addMaterial<Light>(
        "area light",
        &white_tex,
        12.0f,
        Color(1)
    );
    Material& sun_mat = scene.addMaterial<Light>(
        "sun",
        &sun_tex,
        5.0f,
        Color(1)
    );

    auto addMesh = [&scene](
        const std::string& name,
        Material& material,
        const std::string& model,
        const Vec3& position,
        const Vec3& rotation = Vec3(0),
        const Vec3& scale = Vec3(1)
    ) -> Mesh& {
        Mesh& mesh = static_cast<Mesh&>(
            scene.addObject<Mesh>(name, &material, model)
        );
        Transform t = mesh.getTransform();
        t.scale(scale).rotate(rotation).move(position);
        mesh.setTransform(t);
        return mesh;
    };

    auto addSphere = [&scene](
        const std::string& name,
        Material& material,
        float radius,
        const Vec3& position
    ) -> Sphere& {
        Sphere& sphere = static_cast<Sphere&>(
            scene.addObject<Sphere>(name, &material, radius)
        );
        Transform t = sphere.getTransform();
        t.move(position);
        sphere.setTransform(t);
        return sphere;
    };

    addMesh("floor", floor_mat, plane_model, Vec3(0), Vec3(0), Vec3(5, 1, 5));

    addMesh(
        "back wall",
        wall_mat,
        plane_model,
        Vec3(0, 1.6f, -1.75f),
        Vec3(pi / 2.0f, 0, 0),
        Vec3(2.5f, 1, 2.5f)
    );
    addMesh(
        "left wall",
        wall_mat,
        plane_model,
        Vec3(-2.2f, 1.0f, 0.5f),
        Vec3(0, 0, -pi / 2.0f),
        Vec3(2.5f, 1, 2.5f)
    );
    addMesh(
        "right wall",
        wall_mat,
        plane_model,
        Vec3(2.2f, 1.0f, 0.5f),
        Vec3(0, 0, pi / 2.0f),
        Vec3(2.5f, 1, 2.5f)
    );

    addMesh(
        "area light",
        area_light_mat,
        plane_model,
        Vec3(0, 2.3f, -0.2f),
        Vec3(pi, 0, 0),
        Vec3(1.0f, 1, 0.7f)
    );

    addMesh(
        "bunny",
        bunny_mat,
        bunny_model,
        Vec3(0, -0.03f, -0.90f),
        Vec3(0, 0.35f, 0),
        Vec3(0.12f)
    );

    addMesh(
        "teapot",
        teapot_mat,
        teapot_model,
        Vec3(0.73f, 0.0f, -0.05f),
        Vec3(0, -0.4f, 0),
        Vec3(0.13f)
    );

    addMesh(
        "dodecahedron",
        dodecahedron_mat,
        dodecahedron_model,
        Vec3(0.73f, 0.21f, 0.75f),
        Vec3(0.2f, 0.4f, 0),
        Vec3(0.16f)
    );

    addMesh(
        "dice cube",
        dice_mat,
        cube_uv_model,
        Vec3(0.1f, 0.22f, 0.75f),
        Vec3(0.0f, 0.5f, 0.0f),
        Vec3(0.22f)
    );

    addMesh(
        "smoke box",
        smoke_mat,
        cube_model,
        Vec3(1.12f, 0.42f, -0.78f),
        Vec3(0, 0.35f, 0),
        Vec3(0.28f, 0.42f, 0.28f)
    );

    addSphere("glass sphere", glass_mat, 0.34f, Vec3(-0.72f, 0.34f, -0.05f));

    addSphere("earth", earth_mat, 0.24f, Vec3(-0.62f, 0.24f, 0.70f));

    addSphere("sun", sun_mat, 0.26f, Vec3(-1.15f, 1.55f, -0.90f));
}

int main() {
    Camera camera(2.0f, Vec3(0.0f, 1.0f, 2.5f), Vec3(0.0f, 1.0f, 0.0f));

    Scene scene;

    demo(scene, camera);
    // cornelBox(scene, camera);

    scene.buildBVH();

    Gui gui(camera, scene, 300, 300);
    gui.run();
}
