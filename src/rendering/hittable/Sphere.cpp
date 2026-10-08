#include "Sphere.h"
#include "misc/floatcmp.h"
#include <cmath>

Sphere::Sphere(
    const std::string name,
    Material* material,
    const Vec3& center,
    float radius
):
    Hittable(name, material),
    center(center),
    radius(radius) {}

HitInfo Sphere::hit(const Ray& ray) const {
    HitInfo info;

    Ray local_ray = ray;
    local_ray.origin = transform.pointApplyInverse(local_ray.origin);
    local_ray.direction = transform.dirApplyInverse(local_ray.direction);

    Vec3 offset_origin = local_ray.origin - center;

    float a = local_ray.direction.length_squared();
    float half_b = -offset_origin.dot(local_ray.direction);
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
    info.hit_point = local_ray.point(dist);

    Vec3 outward_normal = (info.hit_point - center) / radius;
    info.front_face = local_ray.direction.dot(outward_normal) < 0;

    if(info.front_face)
        info.normal = outward_normal;
    else
        info.normal = -outward_normal;

    info.material = material;
    info.hit_point = transform.pointApply(info.hit_point);
    info.distance = (info.hit_point - ray.origin).dot(ray.direction);
    info.normal = transform.normalApply(info.normal);

    return info;
}

AABB Sphere::getLocalAABB() const {
    return AABB({center + Vec3(radius), center - Vec3(radius)});
}

float& Sphere::getRadius() {
    return radius;
}
