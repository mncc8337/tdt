#include "RawTriangle.h"
#include "misc/floatcmp.h"

RawTriangle::RawTriangle(
    const Vec3& v0,
    const Vec3& v1,
    const Vec3& v2
):
    v0(v0),
    v1(v1),
    v2(v2),
    uv0(Vec2(0)),
    uv1(Vec2(0)),
    uv2(Vec2(0)) {}

RawTriangle::RawTriangle(
    const Vec3& v0,
    const Vec3& v1,
    const Vec3& v2,
    const Vec2& uv0,
    const Vec2& uv1,
    const Vec2& uv2
):
    v0(v0),
    v1(v1),
    v2(v2),
    uv0(uv0),
    uv1(uv1),
    uv2(uv2) {}

AABB getNodeAABB(const RawTriangle& tri) {
    AABB ret = AABB({tri.v0, tri.v1, tri.v2});
    ret.minp -= Vec3(EPSILON);
    ret.maxp += Vec3(EPSILON);
    return ret;
}
