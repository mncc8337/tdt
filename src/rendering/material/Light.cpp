#include "Light.h"

Light::Light(Texture* texture, const float emission_strength, const Color color):
    Material(texture),
    emission_strength(emission_strength),
    color(color) {}

Color Light::emitted(const Vec2 uv, const Vec3 hit_point) const {
    return color * emission_strength;
}
