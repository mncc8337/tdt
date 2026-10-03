#include "Scene.h"

Hittable& Scene::add_object(std::unique_ptr<Hittable> object) {
    objects.push_back(std::move(object));
    return *objects.back().get();
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
    HitInfo min_rec;
    float min_dist = 3.042e38;

    for(const std::unique_ptr<Hittable>& object: objects) {
        HitInfo rec = object->hit(ray);
        if(DID_HIT(rec) and rec.distance < min_dist) {
            min_rec = rec;
            min_dist = rec.distance;
        }
    }

    return min_rec;
}
