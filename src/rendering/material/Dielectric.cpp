#include "Dielectric.h"
#include "misc/RNG.h"
#include "misc/floatcmp.h"

static float reflectance(float cosine, float ri) {
    // use Schlick's approximation for reflectance.
    float r0 = (1-ri) / (1+ri);
    r0 *= r0;
    float icos = 1 - cosine;
    return r0 + (1 - r0) * icos * icos * icos * icos * icos;
}

Dielectric::Dielectric(Texture* texture, Medium* medium):
    Material(texture, medium) {}

ScatterResult Dielectric::scatter(
    Ray& ray,
    Color& attenuation,
    const HitInfo& rec,
    const Medium& origin_medium
) const {
    Vec3 dir = ray.direction.normalized();

    float ior_ratio = origin_medium.getIOR() / medium->getIOR();
    if(!rec.front_face) {
        ior_ratio = 1.0f / ior_ratio;
    }

    float cos_theta = std::min(-dir.dot(rec.normal), 1.0f);
    bool should_reflect = reflectance(cos_theta, ior_ratio) > RNG::uniform();
    bool cannot_refract = ior_ratio * ior_ratio * (1.0 - cos_theta * cos_theta) > 1.0;

    if((cannot_refract or should_reflect) and !fequal(ior_ratio, 1.0f)) {
        // reflection keeps the ray in its current medium
        ray.direction = dir.reflection(rec.normal);
        ray.origin = rec.hit_point + rec.normal * RAY_ORIGIN_OFFSET;
        return ScatterResult::Scattered;
    }

    // refraction crosses the boundary
    ray.direction = dir.refraction(rec.normal, ior_ratio);
    ray.origin = rec.hit_point - rec.normal * RAY_ORIGIN_OFFSET;
    return ScatterResult::Refracted;
}
