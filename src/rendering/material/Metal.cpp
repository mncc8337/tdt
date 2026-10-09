#include "Metal.h"
#include "misc/floatcmp.h"
#include "misc/RNG.h"

Metal::Metal(Texture* texture, float roughness):
    Material(texture, nullptr),
    roughness(roughness) {}

ScatterResult Metal::scatter(
    Ray& ray,
    Color& attenuation,
    const HitInfo& rec,
    const Medium& origin_medium
) const {
    Vec3 dir = ray.direction.reflection(rec.normal);

    if(!fequal0(roughness)) {
        Vec3 p = RNG::directionFast();
        dir += p * roughness;
    }

    if(dir.dot(rec.normal) <= 0.0f)
        dir = dir.reflection(rec.normal);

    ray.direction = dir.normalized();
    ray.origin = rec.hit_point + rec.normal * RAY_ORIGIN_OFFSET;

    attenuation = texture->get(rec);

    return ScatterResult::Scattered;
}

float& Metal::getRoughness() {
    return roughness;
}

const float& Metal::getRoughness() const {
    return roughness;
}
