#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "Color.h"

class RayTracer {
private:
    unsigned viewport_width;
    unsigned viewport_height;
    Camera camera;
    std::vector<std::uint32_t> pixels;

public:
    RayTracer(unsigned viewport_width, unsigned viewport_height);

    const std::uint8_t* getData() const;

    const Color trace(Ray& ray) const;

    void render();
};
