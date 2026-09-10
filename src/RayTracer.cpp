#include "RayTracer.h"
#include "Camera.h"

RayTracer::RayTracer(unsigned viewport_width, unsigned viewport_height):
    viewport_width(viewport_width),
    viewport_height(viewport_height),
    camera(viewport_width, viewport_height, 1.0, Vec3(0, 0, 0), Vec3(0, 0, 1)),
    pixels(std::vector<std::uint32_t>(viewport_width * viewport_height)) {}

const std::uint8_t* RayTracer::getData() const {
    return reinterpret_cast<const std::uint8_t*>(pixels.data());
}

const Color RayTracer::trace(Ray& ray) const {
    // TODO
    return Color(1, 0, 0);
}

void RayTracer::render() {
    // TODO: multi threading
    for(unsigned i = 0; i < viewport_width; i++) {
        for(unsigned j = 0; j < viewport_height; j++) {
            Ray ray = camera.getRayAt(i, j);
            Color cl = trace(ray);
            pixels[i + j * viewport_width] = cl.toABGR();
        }
    }
}
