#pragma once
 
#include "Hittable.h"
#include "math/RawTriangle.h"
 
class Triangle : public Hittable {
private:
    RawTriangle triangle;
    AABB getLocalAABB() const override;

public:
    Triangle(
        Material* material,
        const Vec3& v0,
        const Vec3& v1,
        const Vec3& v2
    );

    HitInfo hit(const Ray& ray) const override;

    AABB getAABB() const override;
};
