#include "Material.h"

Material::Material(Texture* texture):
    texture(texture) {}

bool Material::scatter(Ray& ray, Color& attenuation, const HitInfo& rec) const {
    return false;
}

Color Material::emitted(
    const Vec2 uv,
    const Vec3 hit_point
) const {
    return Color(0, 0, 0);
}
