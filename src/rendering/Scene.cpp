#include "Scene.h"

Hittable& Scene::add_object(std::unique_ptr<Hittable> object) {
    objects.push_back(std::move(object));
    Hittable* obj = objects.back().get();
    object_ptrs.push_back(obj);
    return *obj;
}

Material& Scene::add_material(std::unique_ptr<Material> material) {
    materials.push_back(std::move(material));
    return *materials.back().get();
}

Texture& Scene::add_texture(std::unique_ptr<Texture> texture) {
    textures.push_back(std::move(texture));
    return *textures.back().get();
}

HitInfo Scene::get_closest(const Ray& ray) const {
    return bvh_root->hit(ray);
}

void Scene::build_bvh() {
    bvh_root = std::make_unique<BVHNode>(object_ptrs, 0, object_ptrs.size());
}
