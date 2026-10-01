#include "Sphere.h"
#include <cmath>

Sphere::Sphere(const Vec3& center, double radius) : center(center), radius(radius) {}

HitInfo Sphere::hit(const Ray& ray) const {
    HitInfo info;

    Vec3 oc = ray.getOrigin() - center;

    double a = ray.getDirection().dot(ray.getDirection());
    double b = 2.0 * oc.dot(ray.getDirection());
    double c = oc.dot(oc) - radius * radius;

    double discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0){
        return info;
    }

    double sqrtD = std::sqrt(discriminant);

    double t = (-b - sqrtD) / (2.0 * a);

    if (t < 0) {
        t = (-b + sqrtD) / (2.0 * a);
    }

    if (t < 0){
        return info;
    }

    info.distance = t;
    info.hit_point = ray.point(t);

    Vec3 outward_normal =
        (info.hit_point - center) / radius;

    info.front_face =
        ray.getDirection().dot(outward_normal) < 0;

    if (info.front_face)
        info.normal = outward_normal;
    else
        info.normal = outward_normal * -1;

    info.object = const_cast<Sphere*>(this);

    return info;
}
