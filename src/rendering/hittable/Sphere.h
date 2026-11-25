#pragma once
 
#include "Hittable.h"
 
 
class Sphere : public Hittable {
private:
    AABB calculateLocalAABB() const override;

public:
    Sphere(
        const std::string name,
        Material* material
    );

    HitInfo hit(const Ray& ray) const override;
    Vec2 getUV(const HitInfo& rec) const override;
    bool samplePoint(const Vec3& viewpoint, SurfaceSample& sample) const override;
};
