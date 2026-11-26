#include "Sphere.h"
#include "misc/floatcmp.h"
#include "misc/RNG.h"
#include <cmath>
#include <algorithm>
#include <numbers>

Sphere::Sphere(unsigned id, const std::string name, Material* material): Hittable(id, name, material) {
    refreshAABB();
}

HitInfo Sphere::hit(const Ray& ray) const {
    // the sphere's center and radius used is implied Vec3(0) and 1.0f
    // and should be set via the transform prop

    HitInfo info;

    Ray local_ray = ray;
    local_ray.origin = transform.pointApplyInverse(local_ray.origin);
    local_ray.direction = transform.dirApplyInverse(local_ray.direction);

    Vec3 offset_origin = local_ray.origin /* - center */;

    float a = local_ray.direction.length_squared();
    float half_b = -offset_origin.dot(local_ray.direction);
    float c = offset_origin.length_squared() - 1.0f /* * radius * radius */;

    float discriminant = half_b * half_b - a * c;

    if(discriminant < 0) {
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
    if(dist < EPSILON) {
        return info;
    }

    info.distance = dist;
    info.hit_point = local_ray.point(dist);

    // unit sphere centered at the local origin
    Vec3 outward_normal = info.hit_point;
    info.front_face = local_ray.direction.dot(outward_normal) < 0;
    if(info.front_face)
        info.normal = outward_normal;
    else
        info.normal = -outward_normal;

    info.material = material;
    info.hittable = this;
    info.hit_point = transform.pointApply(info.hit_point);
    info.distance = (info.hit_point - ray.origin).dot(ray.direction);
    info.normal = transform.normalApply(info.normal);

    return info;
}

Vec2 Sphere::getUV(const HitInfo& rec) const {
    const float pi = std::numbers::pi_v<float>;
    Vec3 n_prime = transform.getInverseRotationMatrix() * rec.normal;
    float theta = std::acos(std::clamp(n_prime.y, -1.0f, 1.0f)); // clamping to prevent nan
    float phi = std::atan2(n_prime.z, n_prime.x) + pi;

    return Vec2(
        phi / (2.0f * pi),
        theta / pi
    );
}

bool Sphere::samplePoint(const Vec3& viewpoint, SurfaceSample& sample) const {
    // this example assumes a uniformly scaled sphere. a non-uniform scale
    // turns the sphere into an ellipsoid, which needs a different sampler.
    Vec3 scale = transform.getSize();
    if(!fequal(scale.x, scale.y) || !fequal(scale.y, scale.z))
        return false;

    Vec3 center = transform.pointApply(Vec3(0));
    float world_radius = /* radius * */ scale.x;
    if(world_radius <= EPSILON)
        return false;

    Vec3 to_center = center - viewpoint;
    float dist2 = to_center.length_squared();
    float radius2 = world_radius * world_radius;
    if(dist2 <= radius2 + EPSILON)
        return false;

    // sample a direction uniformly inside the cone subtended by the sphere.
    float cos_theta_max = std::sqrt(std::max(0.0f, 1.0f - radius2 / dist2));
    Vec3 axis = to_center / std::sqrt(dist2);

    // build an orthonormal basis around axis.
    Vec3 helper = std::abs(axis.x) > 0.9f ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
    Vec3 tangent = axis.cross(helper).normalized();
    Vec3 bitangent = axis.cross(tangent);

    const float& pi = std::numbers::pi_v<float>;

    float u1 = RNG::uniform();
    float u2 = RNG::uniform();
    float cos_theta = 1.0f + u1 * (cos_theta_max - 1.0f);
    float sin_theta = std::sqrt(std::max(0.0f, 1.0f - cos_theta * cos_theta));
    float phi = 2.0f * pi * u2;

    Vec3 direction =
        tangent * (std::cos(phi) * sin_theta) +
        bitangent * (std::sin(phi) * sin_theta) +
        axis * cos_theta;

    Ray ray(viewpoint, direction);
    HitInfo hit = this->hit(ray);
    if(!hit.didHit())
        return false;

    Vec3 outward_normal = (hit.hit_point - center).normalized();
    float dist = (hit.hit_point - viewpoint).length();
    float cos_light = std::abs(outward_normal.dot(direction));
    if(dist <= EPSILON || cos_light <= EPSILON)
        return false;

    float solid_angle =
        2.0f * pi * (1.0f - cos_theta_max);
    if(solid_angle <= EPSILON)
        return false;

    // the cone sampler works in solid angle. store the equivalent area density
    // so scene::samplelight can convert it back with r^2 / cos(theta).
    sample.sample = hit.hit_point;
    sample.sample_normal = outward_normal;
    sample.area_pdf = (1.0f / solid_angle) * cos_light / (dist * dist);
    return true;
}

AABB Sphere::calculateLocalAABB() const {
    return AABB({/* center + */ Vec3(1.0f /* * radius */), /* center */ - Vec3(1.0f /* * radius */)});
}
