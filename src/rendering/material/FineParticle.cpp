#include "FineParticle.h"
#include "misc/floatcmp.h"

FineParticle::FineParticle(const std::string name, Texture* texture, Homogeneous* medium):
    Material(name, texture, medium) {}

ScatterResult FineParticle::scatter(
    Ray& ray,
    Color& attenuation,
    const HitInfo& rec,
    const Medium& origin_medium
) const {
    // this would bring the ray inside of the medium if the ray is not in the same medium
    // or outside of the medium if the ray already is
    ray.origin = rec.hit_point - rec.normal * RAY_ORIGIN_OFFSET;
    // FIXME:
    // Refracted is misleading here. the ray does not refracted, it moved to another medium
    // consider rename it to VolumeChanged, it also fits the result of a transparent material
    return ScatterResult::Refracted;
}

Color FineParticle::emitted(const HitInfo& rec) const {
    return Color(0);
}
