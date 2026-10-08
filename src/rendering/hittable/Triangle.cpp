#include "Triangle.h"
#include "misc/floatcmp.h"

Triangle::Triangle(
    const std::string name,
    Material* material,
    const Vec3& v0,
    const Vec3& v1,
    const Vec3& v2
):
    Hittable(name, material),
    triangle(v0, v1, v2) {}

HitInfo Triangle::hit(const Ray& ray) const {
    HitInfo info;

    Ray local_ray = ray;
    local_ray.origin = transform.pointApplyInverse(local_ray.origin);
    local_ray.direction = transform.dirApplyInverse(local_ray.direction);

    Vec3 tuv;
    if(!triangle.hit(local_ray, EPSILON, FAR_DISTANCE, tuv)) {
        return info;
    }

    info.distance = tuv.x;
    info.hit_point = local_ray.point(tuv.x);

    Vec3 edge1 = triangle.v1 - triangle.v0;
    Vec3 edge2 = triangle.v2 - triangle.v0;
    Vec3 outward_normal = edge1.cross(edge2).normalized();

    info.front_face = local_ray.direction.dot(outward_normal) < 0.0f;
    info.normal = info.front_face ? outward_normal : -outward_normal;

    info.material = material;
    info.hit_point = transform.pointApply(info.hit_point);
    info.distance = (info.hit_point - ray.origin).dot(ray.direction);
    info.normal = transform.normalApply(info.normal);


    float u = tuv.y;
    float v = tuv.z;
    float w = 1.0f - u - v;

    info.uv.x = w * triangle.uv0.x + u * triangle.uv1.x + v * triangle.uv2.x;
    info.uv.y = w * triangle.uv0.y + u * triangle.uv1.y + v * triangle.uv2.y;

    return info;
}

AABB Triangle::getLocalAABB() const {
    return AABB({triangle.v0, triangle.v1, triangle.v2});
}

AABB Triangle::getAABB() const {
    return AABB({
        transform.pointApply(triangle.v0),
        transform.pointApply(triangle.v1),
        transform.pointApply(triangle.v2)
    });
}
