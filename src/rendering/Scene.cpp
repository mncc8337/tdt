#include "Scene.h"
#include "Material.h"
#include <memory>

Hittable& Scene::addObject(std::unique_ptr<Hittable> object) {
    objects.push_back(std::move(object));
    Hittable* obj = objects.back().get();
    return *obj;
}

Material& Scene::addMaterial(std::unique_ptr<Material> material) {
    materials.push_back(std::move(material));
    return *materials.back().get();
}

Texture& Scene::addTexture(std::unique_ptr<Texture> texture) {
    textures.push_back(std::move(texture));
    return *textures.back().get();
}

Medium& Scene::addMedium(std::unique_ptr<Medium> medium) {
    mediums.push_back(std::move(medium));
    return *mediums.back().get();
}

Medium*& Scene::getGlobalMedium() {
    return global_medium;
}

const Medium*const& Scene::getGlobalMedium() const {
    return global_medium;
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

    if(!bvh_root) return {};

    return bvh_root->traverse(ray, hitFunc);
}

void Scene::buildBVH() {
    if(objects.empty()) {
        bvh_root.reset();
        return;
    }

    bvh_root = std::make_unique<BVHNode<std::unique_ptr<Hittable>>>(objects, 0, objects.size());
}
