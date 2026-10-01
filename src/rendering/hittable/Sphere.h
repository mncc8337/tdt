#pragma once
 
#include "Hittable.h"
 
 
class Sphere : public Hittable {
private:
    Vec3 center;
    double radius;

public:
    Sphere(const Vec3& center, double radius);
    HitInfo hit(const Ray& ray) const override;
};
 
