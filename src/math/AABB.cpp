#include "AABB.h"
#include <algorithm>

AABB::AABB(const Vec3 p): minp(p), maxp(p), centroid(p) {}

AABB::AABB(const std::initializer_list<Vec3> points) {
    minp = *points.begin();
    maxp = minp;
    centroid = minp;

    for(auto p: points) {
        for(unsigned i = 0; i < 3; i++) {
            minp[i] = std::min(minp[i], p[i]);
            maxp[i] = std::max(maxp[i], p[i]);
        }
    }

    centroid = (minp + maxp) / 2;
}

void AABB::extend(const Vec3& p) {
    for(unsigned i = 0; i < 3; i++) {
        minp[i] = std::min(minp[i], p[i]);
        maxp[i] = std::max(maxp[i], p[i]);
    }
    centroid = (minp + maxp) / 2;
}

void AABB::extend(const AABB& aabb) {
    for(unsigned i = 0; i < 3; i++) {
        minp[i] = std::min(minp[i], aabb.minp[i]);
        maxp[i] = std::max(maxp[i], aabb.maxp[i]);
    }
    centroid = (minp + maxp) / 2;
}

AABB AABB::operator +(const Vec3& p) {
    AABB ret = *this;
    ret.extend(p);
    return ret;
}
AABB AABB::operator +(const AABB& aabb) {
    AABB ret = *this;
    ret.extend(aabb);
    return ret;
}

bool AABB::hit(const Ray& ray, float tmin, float tmax) const {
    for(int a = 0; a < 3; a++) {
        float invD = 1.0f / ray.direction.axis[a];
        float t0 = (minp[a] - ray.origin[a]) * invD;
        float t1 = (maxp[a] - ray.origin[a]) * invD;

        if(invD < 0.0f) std::swap(t0, t1);

        tmin = t0 > tmin ? t0 : tmin;
        tmax = t1 < tmax ? t1 : tmax;

        if (tmax <= tmin) return false;
    }
    return true;
}
