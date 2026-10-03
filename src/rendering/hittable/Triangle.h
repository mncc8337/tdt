#pragma once
 
#include "Hittable.h"
 
 
class Triangle : public Hittable {
private:
    Vec3 v0, v1, v2;

public:
    Triangle(
        Material* material,
        const Vec3& v0,
        const Vec3& v1,
        const Vec3& v2
    );

    HitInfo hit(const Ray& ray) const override;
};
 
