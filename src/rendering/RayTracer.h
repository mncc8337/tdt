#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "Scene.h"

class RayTracer {
private:
    unsigned viewport_width;
    unsigned viewport_height;

    unsigned max_bounces = 50;

    std::vector<Color> buffer;
    std::vector<std::uint32_t> pixels;

public:
    RayTracer(
        unsigned viewport_width,
        unsigned viewport_height
    );

    const std::uint8_t* getData() const;

    const Color trace(Ray ray, const Scene& scene) const;

    // clears accumulated samples and starts accumulation over
    void reset();

    void render(const Camera& camera, const Scene& scene, unsigned pass);
};
