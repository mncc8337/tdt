#pragma once

#include "Vec3.h"
#include "Ray.h"
#include <vector>

struct AABB {
    AABB(const Vec3 p = Vec3(0));
    AABB(const std::vector<Vec3> points);

    Vec3 minp;
    Vec3 maxp;
    Vec3 centroid;

    void extend(const Vec3& p);
    void extend(const AABB& aabb);

    bool hit(const Ray& ray, float tmin, float tmax) const;

    AABB operator +(const Vec3& p);
    AABB operator +(const AABB& aabb);
};
