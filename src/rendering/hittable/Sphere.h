#pragma once
 
#include "Hittable.h"
 
 
class Sphere : public Hittable {
private:
    float radius;

    AABB calculateLocalAABB() const override;

public:
    Sphere(
        const std::string name,
        Material* material,
        float radius
    );

    HitInfo hit(const Ray& ray) const override;

    const float& getRadius() const;
    void setRadius(const float r);
};
