#include "RayTracer.h"
#include "Camera.h"
#include "HitInfo.h"
#include "Vec3.h"
#include "misc/RNG.h"
#include <algorithm>

RayTracer::RayTracer(
    unsigned viewport_width,
    unsigned viewport_height,
    Camera& camera,
    Scene& scene
):
    viewport_width(viewport_width),
    viewport_height(viewport_height),
    camera(camera),
    scene(scene),
    buffer(viewport_width * viewport_height),
    pixels(viewport_width * viewport_height) {}

const std::uint8_t* RayTracer::getData() const {
    return reinterpret_cast<const std::uint8_t*>(pixels.data());
}

const Color RayTracer::trace(Ray ray) const {
    Color final_color(0, 0, 0);
    Color throughput(1, 1, 1);

    for(unsigned bounces = 0; bounces < max_bounces; bounces++) {
        HitInfo rec = scene.get_closest(ray);
        if(!DID_HIT(rec)) {
            // TODO: impl SkyBox class
            float cosine = ray.getDirection().dot(Vec3(0, 1, 0));
            float interpolate = (cosine + 1) / 2;
            Color sky_color = Vec3(0.98, 0.98, 0.98).lerp(Vec3(0.83, 0.95, 1.0), interpolate);
            final_color += throughput * sky_color;
            break;
        }

        Color emitted = rec.material->emitted(rec.uvx, rec.uvy, rec.hit_point);
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
                throughput.getX(),
                throughput.getY(),
                throughput.getZ()
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

void RayTracer::render(unsigned pass) {
    float viewport_h = 2.0f; 
    float viewport_w = viewport_h * ((float)viewport_width / viewport_height);

    #pragma omp parallel for schedule(dynamic, 1)
    for(int j = 0; j < viewport_height; j++) {
        for(int i = 0; i < viewport_width; i++) {
            float u = (i - viewport_width / 2.0f) / viewport_width;
            float v = (j - viewport_height / 2.0f) / viewport_height;

            float px = u * viewport_w;
            float py = -v * viewport_h;
            Color new_color = trace(camera.getRayAt(px, py));
            
            Color& color = buffer[i + j * viewport_width];
            color += new_color;
            pixels[i + j * viewport_width] = Color(color / (pass + 1)).toABGR();
        }
    }
}
