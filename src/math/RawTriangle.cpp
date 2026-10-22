#include "RawTriangle.h"

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
