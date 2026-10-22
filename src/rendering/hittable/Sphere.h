#pragma once
 
#include "Hittable.h"
 
 
class Sphere : public Hittable {
private:
    float radius;

    AABB getLocalAABB() const override;

public:
    Sphere(
        const std::string name,
        Material* material,
        float radius
    );

    HitInfo hit(const Ray& ray) const override;

    float& getRadius();
};
