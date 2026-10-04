#include "AABB.h"
#include <algorithm>

AABB::AABB(Vec3 p): minp(p), maxp(p), centroid(p) {}

void AABB::extend(const Vec3& p) {
    for(unsigned i = 0; i < 3; i++) {
        minp.axis[i] = std::min(minp.axis[i], p.axis[i]);
        maxp.axis[i] = std::max(maxp.axis[i], p.axis[i]);
    }
    centroid = (minp + maxp) / 2;
}

void AABB::extend(const AABB& aabb) {
    for(unsigned i = 0; i < 3; i++) {
        minp.axis[i] = std::min(minp.axis[i], aabb.minp.axis[i]);
        maxp.axis[i] = std::max(maxp.axis[i], aabb.maxp.axis[i]);
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
