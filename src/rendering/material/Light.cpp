#include "Light.h"

Light::Light(Texture* texture, const float emission_strength, const Color color):
    Material(texture),
    emission_strength(emission_strength),
    color(color) {}

Color Light::emitted(const HitInfo& rec) const {
    return texture->get(rec) * emission_strength;
}

float& Light::getEmissionStrength() {
    return emission_strength;
}

const float& Light::getEmissionStrength() const {
    return emission_strength;
}
