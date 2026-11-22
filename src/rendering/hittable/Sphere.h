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
    Vec2 getUV(const HitInfo& rec) const override;
    bool samplePoint(const Vec3& viewpoint, SurfaceSample& sample) const override;

    const float& getRadius() const;
    void setRadius(const float r);
};
