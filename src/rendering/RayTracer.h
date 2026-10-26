#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "Scene.h"

class RayTracer {
private:
    unsigned viewport_width;
    unsigned viewport_height;

    unsigned max_bounces = 10;
    unsigned samples_per_pixel = 4;

    unsigned pass = 0;

    std::vector<Color> buffer;
    std::vector<std::uint32_t> pixels;

public:
    RayTracer(
        unsigned viewport_width,
        unsigned viewport_height
    );

    const unsigned& getPass() const;

    const unsigned& getViewportWidth() const;
    void setViewportWidth(unsigned vw);

    const unsigned& getViewportHeight() const;
    void setViewportHeight(unsigned vh);

    const unsigned& getMaxBounces() const;
    void setMaxBounces(unsigned mb);

    const unsigned& getSamplesPerPixel() const;
    void setSamplesPerPixel(unsigned spp);

    const std::uint8_t* getData() const;

    const Color trace(Ray ray, const Scene& scene) const;

    void reset();

    void render(const Camera& camera, const Scene& scene);
};
