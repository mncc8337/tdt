#include "Sphere.h"
#include <cmath>

Sphere::Sphere(const Vec3& center, float radius) : center(center), radius(radius) {}

HitInfo Sphere::hit(const Ray& ray) const {
    HitInfo info;

    Vec3 oc = ray.getOrigin() - center;

    float a = ray.getDirection().dot(ray.getDirection());
    float half_b = oc.dot(ray.getDirection());
    float c = oc.dot(oc) - radius * radius;

    float discriminant = half_b * half_b - a * c;

    if (discriminant < 0) {
        return info;
    }

    float sqrtD = std::sqrt(discriminant);

    float t = (-half_b - sqrtD) / a;
    if (t < 0.001) {
        t = (-half_b + sqrtD) / a;
    }

    if (t < 0.001) {
        return info;
    }

    info.distance = t;
    info.hit_point = ray.point(t);

    Vec3 outward_normal = (info.hit_point - center) / radius;

    info.front_face =
        ray.getDirection().dot(outward_normal) < 0;

    if (info.front_face)
        info.normal = outward_normal;
    else
        info.normal = outward_normal * -1;

    info.object = this;

    return info;
}
