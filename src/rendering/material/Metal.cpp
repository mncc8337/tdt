#include "Metal.h"
#include "misc/floatcmp.h"
#include "misc/RNG.h"

Metal::Metal(unsigned id, const std::string name, Texture* texture, float roughness):
    Material(id, name, texture, nullptr),
    roughness(roughness) {}

ScatterResult Metal::scatter(
    Ray& ray,
    Color& attenuation,
    const HitInfo& rec,
    const Medium& origin_medium
) const {
    Vec3 spec = ray.direction.reflection(rec.normal);

    if(fequal0(roughness)) {
        ray.direction = spec.normalized();
    } else {
        Vec3 dir = spec.lerp(RNG::cosineHemisphere(spec), roughness);
        Vec3 mirrored = dir.reflection(rec.normal);
        ray.direction = (dir.dot(rec.normal) <= 0.0f ? mirrored : dir) .normalized();
    }

    ray.origin = rec.hit_point + rec.normal * RAY_ORIGIN_OFFSET;

    attenuation = texture->get(rec);

    return ScatterResult::Scattered;
}

Color Metal::emitted(const HitInfo& rec) const {
    return Color(0, 0, 0);
}

float& Metal::getRoughness() {
    return roughness;
}

const float& Metal::getRoughness() const {
    return roughness;
}
