#include "Matte.h"
#include "misc/floatcmp.h"
#include "misc/RNG.h"
#include <numbers>

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

Color Matte::emitted(const HitInfo& rec) const {
    return Color(0, 0, 0);
}

bool Matte::hasNEE() const {
    return true;
}

Color Matte::BRDF(const Vec3& in_dir, const Vec3& out_dir, const HitInfo& rec) const {
    // albedo / pi
    return texture->get(rec) / std::numbers::pi_v<float>;
}
