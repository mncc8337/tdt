#include "Matte.h"
#include "misc/floatcmp.h"
#include "misc/RNG.h"

Matte::Matte(const std::string name, Texture* texture):
    Material(name, texture, nullptr) {}

ScatterResult Matte::scatter(
    Ray& ray,
    Color& attenuation,
    const HitInfo& rec,
    const Medium& origin_medium
) const {
    ray.direction = RNG::cosineHemisphere(rec.normal);
    ray.origin = rec.hit_point + rec.normal * RAY_ORIGIN_OFFSET;

    attenuation = texture->get(rec); 

    return ScatterResult::Scattered;
}
