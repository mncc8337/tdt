#include "Material.h"

Material::Material(Texture* texture):
    texture(texture) {}

Color Material::emitted(
    const Vec2 uv,
    const Vec3 hit_point
) const {
    return Color(0, 0, 0);
}
