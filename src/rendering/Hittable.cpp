#include "Hittable.h"

Hittable::Hittable(const std::string name, Material* material):
    name(name),
    material(material) {}

Hittable::~Hittable() {}

AABB Hittable::getAABB() const {
    AABB local_box = getLocalAABB();

    Vec3 minp = local_box.minp;
    Vec3 maxp = local_box.maxp;
    
    Vec3 corners[8] = {
        {minp.x, minp.y, minp.z},
        {maxp.x, minp.y, minp.z},
        {minp.x, maxp.y, minp.z},
        {maxp.x, maxp.y, minp.z},
        {minp.x, minp.y, maxp.z},
        {maxp.x, minp.y, maxp.z},
        {minp.x, maxp.y, maxp.z},
        {maxp.x, maxp.y, maxp.z}
    };

    AABB transformed(corners[0]);

    for (int i = 0; i < 8; i++) {
        Vec3 world_corner = transform.pointApply(corners[i]);
        transformed.extend(world_corner);
    }

    return transformed;
}

Transform& Hittable::getTransform() {
    return transform;
}

const Transform& Hittable::getTransform() const {
    return transform;
}

Material* Hittable::getMaterial() const {
    return material;
}

std::string& Hittable::getName() {
    return name;
}

const std::string& Hittable::getName() const {
    return name;
}
