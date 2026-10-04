#pragma once

#include "Vec3.h"
#include "Ray.h"

struct RawTriangle {
    RawTriangle(
        const Vec3& v0,
        const Vec3& v1,
        const Vec3& v2
    );

    Vec3 v0, v1, v2;

    bool hit(
        const Ray& ray, 
        float t_min, 
        float t_max, 
        float& t, 
        float& u, 
        float& v
    ) const;
};
