#include "RayTracer.h"
#include "Camera.h"
#include "HitInfo.h"
#include "misc/RNG.h"
#include "misc/ColorUtils.h"

#include <algorithm>

RayTracer::RayTracer(
    unsigned viewport_width,
    unsigned viewport_height
):
    viewport_width(viewport_width),
    viewport_height(viewport_height),
    buffer(viewport_width * viewport_height),
    pixels(viewport_width * viewport_height) {}

const std::uint8_t* RayTracer::getData() const {
    return reinterpret_cast<const std::uint8_t*>(pixels.data());
}

const Color RayTracer::trace(Ray ray, const Scene& scene) const {
    Color final_color(0);
    Color throughput(1);

    for(unsigned bounces = 0; bounces < max_bounces; bounces++) {
        Hittable* obj;
        HitInfo rec = scene.getClosest(ray, obj);
        if(!obj) {
            // TODO: impl SkyBox class
            break;
        }

        Color emitted = rec.material->emitted(rec);
        final_color += throughput * emitted;

        Color attenuation;
        if(!rec.material->scatter(ray, attenuation, rec))
            break; 

        throughput *= attenuation;

        // russian roulette
        if(bounces > 3) {
            // randomly kill rays. rays with higher throughput
            // is less likely to be killed
            float max_channel = std::max({
                throughput.x,
                throughput.y,
                throughput.z
            });
            if (RNG::uniform() > max_channel) {
                break;
            }
            // raise survivied rays' energy by the
            // inverse of the probability of being killed
            throughput /= max_channel; 
        }
    }

    return final_color;
}

void RayTracer::reset() {
    std::fill(buffer.begin(), buffer.end(), Color(0));
    std::fill(pixels.begin(), pixels.end(), 0);
}

void RayTracer::render(const Camera& camera, const Scene& scene, unsigned pass) {
    float viewport_h = 2.0f; 
    float viewport_w = viewport_h * ((float)viewport_width / viewport_height);

    // read once so every pixel of the pass uses the same exposure,
    // a change only takes effect on the next pass
    float exposure = camera.getExposure();

    #pragma omp parallel for schedule(dynamic, 1)
    for(int j = 0; j < viewport_height; j++) {
        for(int i = 0; i < viewport_width; i++) {
            float u = (i - viewport_width / 2.0f) / viewport_width;
            float v = (j - viewport_height / 2.0f) / viewport_height;

            Ray ray = camera.getRayAt(Vec2(u * viewport_w, -v * viewport_h));
            Color new_color = trace(ray, scene);
            buffer[i + j * viewport_width] += new_color;

            Color display = buffer[i + j * viewport_width] / (pass + 1);
            gammaCorrect(exposureTonemap(display, exposure), 2.2f);
            pixels[i + j * viewport_width] = display.toABGR();
        }
    }
}
