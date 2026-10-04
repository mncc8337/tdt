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

bool RawTriangle::hit(
    const Ray& ray, 
    float t_min, 
    float t_max, 
    Vec3& tuv
) const {
    Vec3 edge1 = v1 - v0;
    Vec3 edge2 = v2 - v0;

    Vec3 pvec = ray.direction.cross(edge2);
    float det = edge1.dot(pvec);

    if (fequal0(det)) return false;

    float& t = tuv.x;
    float& u = tuv.y;
    float& v = tuv.z;

    float inv_det = 1.0f / det;
    Vec3 tvec = ray.origin - v0;

    u = tvec.dot(pvec) * inv_det;
    if (u < 0.0f || u > 1.0f) return false;

    Vec3 qvec = tvec.cross(edge1);
    v = ray.direction.dot(qvec) * inv_det;
    if (v < 0.0f || u + v > 1.0f) return false;

    t = edge2.dot(qvec) * inv_det;

    if (t < t_min || t > t_max) return false; 

    return true; 
}
