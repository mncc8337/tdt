#include "Material.h"

Material::Material(Texture* texture, Medium* medium):
    texture(texture),
    medium(medium) {}

ScatterResult Material::scatter(
    Ray& ray,
    Color& attenuation,
    const HitInfo& rec,
    const Medium& origin_medium
) const {
    return ScatterResult::Terminated;
}

Color Material::emitted(const HitInfo& rec) const {
    return Color(0, 0, 0);
}

Texture* Material::getTexture() const {
    return texture;
}

Medium* Material::getMedium() const {
    return medium;
}
