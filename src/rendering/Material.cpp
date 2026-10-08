#include "Material.h"

Material::Material(Texture* texture):
    texture(texture) {}

bool Material::scatter(Ray& ray, Color& attenuation, const HitInfo& rec) const {
    return false;
}

Color Material::emitted(const HitInfo& rec) const {
    return Color(0, 0, 0);
}

Texture* Material::getTexture() const {
    return texture;
}
