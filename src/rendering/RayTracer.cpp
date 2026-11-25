#include "RayTracer.h"
#include "Camera.h"
#include "Material.h"
#include "HitInfo.h"
#include "Medium.h"
#include "misc/RNG.h"
#include "misc/floatcmp.h"

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
    // reset();
}

const unsigned& RayTracer::getSamplesPerPixel() const {
    return samples_per_pixel;
}

void RayTracer::setSamplesPerPixel(unsigned spp) {
    samples_per_pixel = spp;
    // reset();
}

const bool& RayTracer::getLazy() const {
    return lazy;
}
const void RayTracer::setLazy(bool l) {
    lazy = l;
}

const std::vector<Color>& RayTracer::getBuffer() const {
    return buffer;
}

const Color RayTracer::trace(Ray ray, const Scene& scene) const {
    Color final_color(0);
    Color throughput(1);

    // light whose emission was already added by next-event estimation at the
    // previous bounce. tracking the light keeps the double-count guard correct
    // when later bounces hit a different light.
    const Hittable* directly_sampled_light = nullptr;

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

        // no surface hit means the ray travels towards the sky, so use an
        // effectively infinite distance instead of the -1 sentinel from
        // HitInfo (a negative distance would amplify the throughput)
        const float travel_distance = rec.didHit() ? rec.distance : FAR_DISTANCE;

        // try to transmit to the object (or to the sky if nothing was hit)
        float transmitted_distance = current_medium->transmit(ray, throughput, travel_distance);

        // FIXME:
        // we could try to transmit through the medium with inf distance first
        // to get the maximum distance we could travel without being intercept by the medium
        // then we could use the transmitted distance as the higher bound for our intersection tests
        // which could reduce the number of tests we need to perform

        if(transmitted_distance < travel_distance) {
            // failed to transmit to the object
            // got scattered by the medium in the way
            // direct light sampling is only implemented for surfaces
            // so allow emission to be collected on the next light hit.
            directly_sampled_light = nullptr;
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

        // only skip emission from the light whose contribution was already
        // added by NEE at the previous bounce
        // other lights still need to be collected when the continued path
        // happens to hit them
        bool emission_already_accounted = (obj == directly_sampled_light);
        directly_sampled_light = nullptr;
        if(!emission_already_accounted) {
            final_color += throughput * rec.material->emitted(rec);
        }

        // next-event estimation for materials with an evaluable BSDF
        // this is the simplest form: one light sample, conditional pdf
        if(rec.material->hasNEE()) {
            LightSample light_sample;
            if(scene.sampleLight(rec.hit_point, light_sample)) {
                float cos_theta = std::max(
                    0.0f,
                    rec.normal.dot(light_sample.light_direction)
                );

                if(cos_theta > 0.0f && light_sample.pdf > 0.0f) {
                    Ray shadow_ray(
                        rec.hit_point + rec.normal * RAY_ORIGIN_OFFSET,
                        light_sample.light_direction
                    );
                    Hittable* blocker = nullptr;
                    HitInfo shadow_hit = scene.getClosest(shadow_ray, blocker);

                    bool visible = (
                        !shadow_hit.didHit()
                        or (
                            blocker == light_sample.light
                            // change epsilon to 1e-4
                            // the main distance difference is going to be RAY_ORIGIN_OFFSET = 1e-5
                            // (since the sample ray is offseted by that ammount from the object surface)
                            // so epsilon need to be larger than that for the comparison to be true
                            // builtin EPSILON = 1e-6 is smaller than that, so replaced
                            and fequal(shadow_hit.distance, light_sample.light_distance, 1e-4)
                        )
                    );

                    if(visible) {
                        Color att = rec.material->BRDF(light_sample.light_direction, -ray.direction, rec);
                        att *= light_sample.emitted_radiance * cos_theta / light_sample.pdf;
                        final_color += throughput * att;

                        // remember which light was counted for so the next bounce
                        // does not count its emission again
                        directly_sampled_light = light_sample.light;
                    }
                }
            }
        }

        // surface scattering weight
        // materials that absorb at the surface overwrite it
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
            directly_sampled_light = nullptr;
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
            // render in checkerboard pattern
            // reduce render time in half
            // only sensible when modifying the scene
            // through the gui since it gives the least visual delay
            if(lazy and ((i + j * viewport_width + pass + j % 2) % 2))
                continue;

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

            // since only half of the frame is rendered
            // we double the ammount the other half collected to compensate
            if(lazy)
                accumulate *= 2.0f;

            buffer[i + j * viewport_width] += accumulate / spp;
        }
    }

    pass++;
}
