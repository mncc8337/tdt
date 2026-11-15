#pragma once

#include "Vec3.h"
#include "Vec2.h"
#include "AABB.h"

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

    Vec3 origin;
    Vec3 edge1, edge2;
    Vec2 uv0, uv1, uv2;
};

// for the BVHNode template
AABB getNodeAABB(const RawTriangle& tri);
