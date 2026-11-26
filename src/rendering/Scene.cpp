#include "Scene.h"
#include "misc/RNG.h"
#include "misc/floatcmp.h"

#include <cmath>
#include <system_error>

const Medium& Scene::getGlobalMedium() const {
    // keep a valid fallback so an empty scene (or a removed global medium)
    // is still safe to trace.
    static const Medium fallback(0, "vacuum");
    return global_medium ? *global_medium : fallback;
}

void Scene::setGlobalMedium(Medium& medium) {
    global_medium = &medium;
}

void Scene::remove_object(const Hittable* object) {
    Hittable* target = findOwned(objects, object);
    if(!target) return;

    std::erase(lights, target);
    eraseOwnedIf(objects, [target](Hittable* owned) { return owned == target; });
    buildBVH();
}

void Scene::set_transform(const Hittable* object, const Transform& transform) {
    Hittable* target = findOwned(objects, object);
    if(!target) return;

    target->setTransform(transform);
    buildBVH();
}

void Scene::set_material(const Hittable* object, const Material* material) {
    Hittable* target = findOwned(objects, object);
    if(!target) return;

    Material* next = findOwned(materials, material);
    if(!next) return;

    target->setMaterial(next);

    std::erase(lights, target);
    if(next and next->isEmissive())
        lights.push_back(target);
}

void Scene::remove_material(const Material* material) {
    Material* target = findOwned(materials, material);
    if(!target) return;

    // FIXME:
    // throw an exception when the material is used by an object
    // for now just silently drop the request
    for(const auto& object : objects) {
        if(object->getMaterial() == target)
            return;
    }

    eraseOwnedIf(materials, [target](Material* owned) { return owned == target; });
}

void Scene::set_texture(const Material* material, const Texture* texture) {
    Material* target = findOwned(materials, material);
    if(!target) return;

    Texture* next = findOwned(textures, texture);
    if(!next) return;

    target->setTexture(next);
}

void Scene::set_medium(const Material* material, const Medium* medium) {
    Material* target = findOwned(materials, material);
    if(!target) return;

    Medium* next = findOwned(mediums, medium);
    if(!next) return;

    target->setMedium(next);
}

void Scene::remove_texture(const Texture* texture) {
    Texture* target = findOwned(textures, texture);
    if(!target) return;

    // FIXME:
    // throw an exception when the texture is used by a material
    // for now just silently drop the request
    for(const auto& material : materials) {
        if(material->getTexture() == target)
            return;
    }

    eraseOwnedIf(textures, [target](Texture* owned) { return owned == target; });
}

void Scene::remove_medium(const Medium* medium) {
    Medium* target = findOwned(mediums, medium);
    if(!target) return;

    // FIXME:
    // throw an exception when the material is used by an object
    // for now just silently drop the request
    for(const auto& material : materials) {
        if(material->getMedium() == target)
            return;
    }

    // FIXME:
    // throw an exception when trying to delte the global medium
    if(global_medium == target)
        return;

    eraseOwnedIf(mediums, [target](Medium* owned) { return owned == target; });
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

Scene::~Scene() {
    clear();
}

void Scene::clear() {
    objects.clear();
    materials.clear();
    textures.clear();
    mediums.clear();
    lights.clear();
    global_medium = nullptr;
    bvh_root.reset();
    next_id = 0;

    std::error_code ec;
    for(const auto& path : temp_assets)
        std::filesystem::remove(path, ec);
    temp_assets.clear();
}

