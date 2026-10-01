#include "Sphere.h"
#include <cmath>

#define EPSILON 1e-6

inline bool _equal_zero(float a) {
    return std::abs(a) < EPSILON;
}

Sphere::Sphere(const Vec3& center, float radius) : center(center), radius(radius) {}

HitInfo Sphere::hit(const Ray& ray) const {
    HitInfo info;

    Vec3 offset_origin = ray.getOrigin() - center;

    // since ray.getDirection() is always normalised
    // the a term simply equals to 1.0
    // so we can get rid of it from any multiplication/division
    float half_b = offset_origin.dot(ray.getDirection());
    float c = offset_origin.length_squared() - radius * radius;

    // determine whether the ray origin is in the sphere or not
    // if offset_origin.length - radius is negative then offset_origin.length^2 - radius^2 is also negative. it is the same if the result is positive or zero
    // so we can reuse c instead of having to calculate the length of offset_origin
    bool inside_object = c < 0;

    float discriminant = half_b * half_b - c;

    if (discriminant < 0) {
        return info;
    }

    float sqrt_d = std::sqrt(discriminant);

    float dist = half_b - sqrt_d;
    if (dist < EPSILON) {
        dist = half_b + sqrt_d;
    }

    // prevent back-intersection
    // also ignore the case when the ray lies too close to the sphere surface
    // to avoid nan handling complications
    if (dist < EPSILON) {
        return info;
    }

    info.distance = dist;
    info.hit_point = ray.point(dist);

    Vec3 outward_normal = (info.hit_point - center) / radius;

    info.front_face = not inside_object;

    if (info.front_face)
        info.normal = outward_normal;
    else
        info.normal = -outward_normal;

    info.object = this;

    return info;
}
