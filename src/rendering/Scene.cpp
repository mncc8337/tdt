#include "Scene.h"
#include "HitInfo.h"
#include <memory>

void Scene::add_object(std::unique_ptr<Hittable> object) {
    objects.push_back(std::move(object));
}

HitInfo Scene::get_closest(const Ray& ray) const {
    HitInfo min_rec;
    min_rec.distance = 3.042e38;

    for(const std::unique_ptr<Hittable>& object: objects) {
        HitInfo rec = object->hit(ray);
        if(rec.distance >= 0.0 and rec.distance < min_rec.distance) {
            min_rec = rec;
        }
    }

    return min_rec;
}
