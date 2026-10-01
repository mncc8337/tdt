#include "Triangle.h"
#include <cmath>

Triangle::Triangle(const Vec3& v0, const Vec3& v1, const Vec3& v2) : v0(v0), v1(v1), v2(v2) {}

HitInfo Triangle::hit(const Ray& ray) const {
    HitInfo info;

    const double epsilon = 1e-8;

    Vec3 edge1 = v1 - v0;
    Vec3 edge2 = v2 - v0;

    Vec3 pvec = ray.getDirection().cross(edge2);
    double det = edge1.dot(pvec);

    if (std::fabs(det) < epsilon){
        return info;
    }

    double invDet = 1.0 / det;

    Vec3 tvec = ray.getOrigin() - v0;

    double u = tvec.dot(pvec) * invDet;
    if (u < 0.0 || u > 1.0){
        return info;
    }

    Vec3 qvec = tvec.cross(edge1);

    double v = ray.getDirection().dot(qvec) * invDet;
    if (v < 0.0 || u + v > 1.0){
        return info;
    }

    double t = edge2.dot(qvec) * invDet;

    if (t < 0){
        return info;
    }

    info.distance = t;
    info.hit_point = ray.point(t);

    Vec3 outward_normal = edge1.cross(edge2).normalized();


    if (det < 0)
        outward_normal = outward_normal * -1;

    info.front_face =
        ray.getDirection().dot(outward_normal) < 0;

    if (info.front_face)
        info.normal = outward_normal;
    else
        info.normal = outward_normal * -1;

    info.object = const_cast<Triangle*>(this);

    return info;
}
