#pragma once

#include <cstdint>
#include <vector>

#include "Color.h"
#include "Camera.h"
#include "Scene.h"

class RayTracer {
private:
    unsigned viewport_width;
    unsigned viewport_height;

    unsigned max_bounces = 10;

    Camera& camera;
    Scene& scene;

    std::vector<Color> buffer;
    std::vector<std::uint32_t> pixels;

public:
    RayTracer(unsigned viewport_width, unsigned viewport_height, Camera& camera, Scene& scene);

    const std::uint8_t* getData() const;

    const Color trace(Ray ray) const;

    void render(unsigned pass);
};
