#pragma once

#include "Vec3.h"
#include "Vec2.h"
#include "Ray.h"

struct RawTriangle {
    RawTriangle(
        const Vec3& v0,
        const Vec3& v1,
        const Vec3& v2
    );

    RawTriangle(
        const Vec3& v0,
        const Vec3& v1,
        const Vec3& v2,
        const Vec2& uv0,
        const Vec2& uv1,
        const Vec2& uv2
    );

    Vec3 v0, v1, v2;
    Vec2 uv0, uv1, uv2;

    bool hit(
        const Ray& ray, 
        float t_min, 
        float t_max, 
        Vec3& tuv
    ) const;
};
