#include "RayTracer.h"
#include "Camera.h"
#include "Material.h"
#include "HitInfo.h"
#include "Medium.h"
#include "misc/RNG.h"

#include <algorithm>
#include <array>
#include <cmath>

RayTracer::RayTracer(
    unsigned viewport_width,
    unsigned viewport_height
):
    viewport_width(viewport_width),
    viewport_height(viewport_height),
    buffer(viewport_width * viewport_height),
    pass(0){}

const unsigned& RayTracer::getPass() const {
    return pass;
}

const unsigned& RayTracer::getViewportWidth() const {
    return viewport_width;
}

void RayTracer::setViewportWidth(unsigned vw) {
    viewport_width = vw;
    buffer.resize(viewport_width * viewport_height);
    reset();
}

const unsigned& RayTracer::getViewportHeight() const {
    return viewport_height;
}

void RayTracer::setViewportHeight(unsigned vh) {
    viewport_height = vh;
    buffer.resize(viewport_width * viewport_height);
    reset();
}

const unsigned& RayTracer::getMaxBounces() const {
    return max_bounces;
}

void RayTracer::setMaxBounces(unsigned mb) {
    max_bounces = mb;
    reset();
}

const unsigned& RayTracer::getSamplesPerPixel() const {
    return samples_per_pixel;
}

void RayTracer::setSamplesPerPixel(unsigned spp) {
    samples_per_pixel = spp;
    reset();
}

const std::vector<Color>& RayTracer::getBuffer() const {
    return buffer;
}

const Color RayTracer::trace(Ray ray, const Scene& scene) const {
    Color final_color(0);
    Color throughput(1);

    // use std::array instead of std::stack to reduce dynamic allocation
    // bounded by max_bounces: each bounce can push at most one medium
    // since cpp does not accept variable templating, we can only hardcode it at 64
    std::array<const Medium*, 64> medium_stack;
    size_t medium_stack_size = 1;
    medium_stack[0] = &scene.getGlobalMedium();

    for(unsigned bounces = 0; bounces < max_bounces; bounces++) {
        // russian roulette
        if(bounces > 3) {
            float max_channel = std::max({
                throughput.x,
                throughput.y,
                throughput.z
            });
            if (RNG::uniform() > max_channel) {
                break;
            }
            throughput /= max_channel; 
        }

        Hittable* obj;
        HitInfo rec = scene.getClosest(ray, obj);

        const Medium* current_medium = medium_stack[medium_stack_size - 1];

        // try to transmit to the object (distance = rec.distance)
        float transmitted_distance = current_medium->transmit(ray, throughput, rec.distance);

        // FIXME:
        // we could try to transmit through the medium with inf distance first
        // to get the maximum distance we could travel without being intercept by the medium
        // then we could use the transmitted distance as the higher bound for our intersection tests
        // which could reduce the number of tests we need to perform

        if(transmitted_distance < rec.distance) {
            // failed to transmit to the object
            // got scattered by the medium in the way
            continue;
        }

        if(!obj) {
            // only hit the skybox if the ray hit neither the objects nor the current medium
            // TODO: impl SkyBox class
            // float ncos = (ray.direction.dot(Vec3(0, 1, 0)) + 1.0f) / 2.0f;
            // Vec3 sky_color = Vec3(1.0f, 0.514f, 0.451f).lerp(
            //     Vec3(1).lerp(Vec3(0.451f, 0.725f, 1.0f), ncos),
            //     ncos
            // );
            // final_color += throughput * sky_color;
            break;
        }

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

        if(result == ScatterResult::Terminated) {
            break;
        } else if(result == ScatterResult::Scattered) {
            throughput *= surface_attenuation;
        } else if(result == ScatterResult::Refracted) {
            Medium* object_medium = rec.material->getMedium();
            if(object_medium) {
                if(rec.front_face) {
                    if(medium_stack_size < medium_stack.size())
                        medium_stack[medium_stack_size++] = object_medium;
                } else if(medium_stack_size > 1 && current_medium == object_medium) {
                    medium_stack_size--;
                }
            }
        }
    }

    return final_color;
}

void RayTracer::reset() {
    std::fill(buffer.begin(), buffer.end(), Color(0));
    pass = 0;
}

void RayTracer::render(const Camera& camera, const Scene& scene) {
    Vec2 world_viewport = Vec2(
        2.0f * ((float)viewport_width / viewport_height),
        2.0f
    );

    int spp_sqrt = std::max(1, int(std::sqrt(samples_per_pixel)));
    int spp = spp_sqrt * spp_sqrt;
    float inv_spp_sqrt = 1.0f / spp_sqrt;

    #pragma omp parallel for schedule(dynamic, 1)
    for(int j = 0; j < viewport_height; j++) {
        for(int i = 0; i < viewport_width; i++) {
            Color accumulate = Color(0);

            // stratified sampling
            for(int sx = 0; sx < spp_sqrt; sx++) {
                for(int sy = 0; sy < spp_sqrt; sy++) {
                    Vec2 jitter = RNG::vec2();
                    float su = (float(sx) + jitter.x) * inv_spp_sqrt;
                    float sv = (float(sy) + jitter.y) * inv_spp_sqrt;
                    Vec2 uv = Vec2(
                        float(i + su) / viewport_width - 0.5f,
                        float(j + sv) / viewport_height - 0.5f
                    );

                    uv.y *= -1;
                    accumulate += trace(camera.getRayAt(uv * world_viewport), scene);
                }
            }

            buffer[i + j * viewport_width] += accumulate / spp;
        }
    }

    pass++;
}
