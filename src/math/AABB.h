#pragma once

#include "Vec3.h"

struct AABB {
    AABB(Vec3 p);

    Vec3 minp;
    Vec3 maxp;
    Vec3 centroid;

    void extend(const Vec3& p);
    void extend(const AABB& aabb);

    AABB operator +(const Vec3& p);
    AABB operator +(const AABB& aabb);
};
