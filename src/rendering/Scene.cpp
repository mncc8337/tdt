#include "Scene.h"
#include "misc/RNG.h"
#include "misc/floatcmp.h"

#include <algorithm>
#include <cmath>

const Medium& Scene::getGlobalMedium() const {
    return *global_medium;
}

void Scene::setGlobalMedium(Medium& medium) {
    global_medium = &medium;
}

const std::vector<std::unique_ptr<Hittable>>& Scene::getObjects() const {
    return objects;
}

const std::vector<std::unique_ptr<Material>>& Scene::getMaterials() const {
    return materials;
}

const std::vector<std::unique_ptr<Texture>>& Scene::getTextures() const {
    return textures;
}

const std::vector<std::unique_ptr<Medium>>& Scene::getMediums() const {
    return mediums;
}

bool Scene::sampleLight(const Vec3& from, LightSample& sample) const {
    if(lights.empty()) return false;

    // uniform light selection for this example
    const Hittable* light = RNG::sample(lights);

    SurfaceSample surface;
    if(!light->samplePoint(from, surface))
        return false;
    if(surface.area_pdf <= 0.0f)
        return false;

    Vec3 to_light = surface.sample - from;
    float dist2 = to_light.length_squared();
    if(dist2 <= EPSILON * EPSILON)
        return false;

    float light_distance = std::sqrt(dist2);
    Vec3 light_direction = to_light / light_distance;

    float cos_light = std::abs(surface.sample_normal.dot(light_direction));
    if(cos_light <= EPSILON)
        return false;

    // convert the area pdf to a solid-angle pdf, then include the
    // probability of selecting this light
    float select_pdf = 1.0f / float(lights.size());
    float pdf = select_pdf * surface.area_pdf * dist2 / cos_light;
    if(pdf <= 0.0f)
        return false;

    // evaluate the emitted radiance at the sampled point
    // tracing the light also gives us its UVs, which textured lights need
    Ray to_light_ray(from, light_direction);
    HitInfo light_hit = light->hit(to_light_ray);
    if(!light_hit.didHit())
        return false;

    sample.light_direction = light_direction;
    sample.light_distance = light_distance;
    sample.pdf = pdf;
    sample.emitted_radiance = light->getMaterial()->emitted(light_hit);
    sample.light = light;
    return true;
}

HitInfo Scene::getClosest(const Ray& ray, Hittable*& obj) const {
    obj = nullptr;

    auto hitFunc = [&](const int primitive_idx, HitInfo& current_closest) {
        const auto& hittable_obj = objects[primitive_idx];
        HitInfo temp_hit = hittable_obj->hit(ray);

        if(temp_hit.didHit()) {
            if(!current_closest.didHit() or temp_hit.distance < current_closest.distance) {
                current_closest = temp_hit;
                obj = hittable_obj.get();
            }
        }
    };

    // use linear search if the object count is small
    if(objects.size() < SCENE_BVH_THRESHOLD) {
        HitInfo closest_hit;
        for(int i = 0; i < objects.size(); i++) {
            hitFunc(i, closest_hit);
        }
        return closest_hit;
    }

    if(!bvh_root) return {};
    return bvh_root->traverse(ray, hitFunc);
}

void Scene::buildBVH() {
    if(objects.empty()) {
        bvh_root.reset();
        return;
    }

    bvh_root = std::make_unique<BVHNode>(objects, 0, objects.size());
}
