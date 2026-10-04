#include "RawTriangle.h"
#include "misc/floatcmp.h"

RawTriangle::RawTriangle(
    const Vec3& v0,
    const Vec3& v1,
    const Vec3& v2
):
    v0(v0),
    v1(v1),
    v2(v2) {}

bool RawTriangle::hit(
    const Ray& ray, 
    float t_min, 
    float t_max, 
    float& t, 
    float& u, 
    float& v
) const {
    Vec3 edge1 = v1 - v0;
    Vec3 edge2 = v2 - v0;

    Vec3 pvec = ray.direction.cross(edge2);
    float det = edge1.dot(pvec);

    if (fequal0(det)) return false;

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
