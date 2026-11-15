#pragma once

#include <initializer_list>
#include "Vec3.h"
#include "Ray.h"

struct AABB {
    AABB(const Vec3 p = Vec3(0));
    AABB(const std::initializer_list<Vec3> points);

    Vec3 minp;
    Vec3 maxp;

    void extend(const Vec3& p);
    void extend(const AABB& aabb);

    bool hit(
        const Ray& ray,
        const Vec3& inv_direction,
        float tmin,
        float tmax
    ) const;

    AABB operator +(const Vec3& p);
    AABB operator +(const AABB& aabb);
};
