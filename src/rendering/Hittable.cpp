#include "Hittable.h"

Hittable::Hittable(const std::string name, Material* material):
    NamedElement(name),
    material(material) {}

Hittable::~Hittable() {}

AABB Hittable::calculateGlobalAABB() const {
    Vec3 minp = local_aabb.minp;
    Vec3 maxp = local_aabb.maxp;

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

    for(int i = 0; i < 8; i++) {
        Vec3 world_corner = transform.pointApply(corners[i]);
        transformed.extend(world_corner);
    }

    return transformed;
}

void Hittable::recomputeAABB() {
    local_aabb = calculateLocalAABB();
    global_aabb = calculateGlobalAABB();
}

AABB Hittable::getAABB() const {
    return global_aabb;
}

const Transform& Hittable::getTransform() const {
    return transform;
}

void Hittable::setTransform(const Transform t) {
    transform = t;
    global_aabb = calculateGlobalAABB();
}

Material* Hittable::getMaterial() const {
    return material;
}
