#include "RayTracer.h"
#include "Camera.h"
#include "Material.h"
#include "HitInfo.h"
#include "misc/RNG.h"
#include "misc/ColorUtils.h"

#include <algorithm>
#include <stack>

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

    std::stack<const Medium*> medium_stack;
    medium_stack.push(scene.getGlobalMedium());

    for(unsigned bounces = 0; bounces < max_bounces; bounces++) {
        Hittable* obj;
        HitInfo rec = scene.getClosest(ray, obj);
        if(!obj) {
            // TODO: impl SkyBox class
            break;
        }

        const Medium* current_medium = medium_stack.top();

        // attenuate the segment the ray just travelled through,
        // then collect light emitted at the surface it landed on
        throughput *= current_medium->transmit(rec.distance);
        final_color += throughput * rec.material->emitted(rec);

        // surface scattering weight.
        // materials that absorb at the surface overwrite it
        // a dielectric leaves it as is, absorption along the path is the medium's job
        Color surface_attenuation(1);
        ScatterResult result = rec.material->scatter(
            ray,
            surface_attenuation,
            rec,
            *current_medium
        );
        if(result == ScatterResult::Terminated)
            break;

        throughput *= surface_attenuation;

        if(result == ScatterResult::Refracted) {
            Medium* object_medium = rec.material->getMedium();
            if(object_medium) {
                if(rec.front_face) {
                    medium_stack.push(object_medium);
                } else if(medium_stack.size() > 1 && current_medium == object_medium) {
                    medium_stack.pop();
                }
            }
        }

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
