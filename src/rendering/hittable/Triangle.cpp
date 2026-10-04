#include "Triangle.h"

Triangle::Triangle(
    Material* material,
    const Vec3& v0,
    const Vec3& v1,
    const Vec3& v2
):
    triangle(v0, v1, v2),
    Hittable(material) {}

HitInfo Triangle::hit(const Ray& ray) const {
    HitInfo info;

    float t, u, v;
    
    if(!triangle.hit(ray, 0.001f, 1e9f, t, u, v)) {
        return info;
    }

    info.distance = t;
    info.hit_point = ray.point(t);

    Vec3 edge1 = triangle.v1 - triangle.v0;
    Vec3 edge2 = triangle.v2 - triangle.v0;
    Vec3 outward_normal = edge1.cross(edge2).normalized();

    info.front_face = ray.direction.dot(outward_normal) < 0.0f;
    info.normal = info.front_face ? outward_normal : outward_normal * -1.0f;

    info.material = material;

    // info.u = u;
    // info.v = v;

    return info;
}

AABB Triangle::getAABB() const {
    return AABB({triangle.v0, triangle.v1, triangle.v2});
}
