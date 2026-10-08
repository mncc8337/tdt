#pragma once
 
#include "Hittable.h"
 
 
class Sphere : public Hittable {
private:
    Vec3 center;
    float radius;

    AABB getLocalAABB() const override;

public:
    Sphere(
        const std::string name,
        Material* material,
        const Vec3& center,
        float radius
    );

    HitInfo hit(const Ray& ray) const override;

    float& getRadius();
};
