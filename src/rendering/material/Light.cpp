#include "Light.h"

Light::Light(
    unsigned id,
    const std::string name,
    Texture* texture,
    const float emission_strength,
    const Color color
):
    Material(id, name, texture, nullptr),
    emission_strength(emission_strength),
    color(color) {}

ScatterResult Light::scatter(
    Ray& ray,
    Color& attenuation,
    const HitInfo& rec,
    const Medium& origin_medium
) const {
    return ScatterResult::Terminated;
}

Color Light::emitted(const HitInfo& rec) const {
    return texture->get(rec) * emission_strength;
}

bool Light::isEmissive() const {
    return true;
}

float& Light::getEmissionStrength() {
    return emission_strength;
}

const float& Light::getEmissionStrength() const {
    return emission_strength;
}

Color& Light::getColor() {
    return color;
}

const Color& Light::getColor() const {
    return color;
}
