#include "Sphere.h"
#include "misc/floatcmp.h"
#include <cmath>

Sphere::Sphere(
    Material* material,
    const Vec3& center,
    float radius
):
    Hittable(material),
    center(center),
    radius(radius) {}

HitInfo Sphere::hit(const Ray& ray) const {
    HitInfo info;

    Vec3 offset_origin = ray.getOrigin() - center;

    float a = ray.getDirection().length_squared();
    float half_b = -offset_origin.dot(ray.getDirection());
    float c = offset_origin.length_squared() - radius * radius;

    float discriminant = half_b * half_b - a * c;

    if (discriminant < 0) {
        return info;
    }

    float sqrt_d = std::sqrt(discriminant);

    float inv_a = 1.0f / a;
    float dist = (half_b - sqrt_d) * inv_a;

    if(dist < EPSILON) {
        dist = (half_b + sqrt_d) * inv_a;
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
    info.front_face = ray.getDirection().dot(outward_normal) < 0;

    if(info.front_face)
        info.normal = outward_normal;
    else
        info.normal = -outward_normal;

    info.material = material;

    return info;
}
