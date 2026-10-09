#include "medium/Homogeneous.h"
#include <cmath>

Homogeneous::Homogeneous(
    const float ior,
    const Color sigma,
    const float density
):
    Medium(ior),
    sigma(sigma),
    density(density) {}

Color Homogeneous::transmit(const float distance) const {
    Color eff_sigma = sigma * density;
    return Color(
        std::exp(-eff_sigma.x * distance),
        std::exp(-eff_sigma.y * distance),
        std::exp(-eff_sigma.z * distance)
    );
}

Color& Homogeneous::getSigma() {
    return sigma;
}

const Color& Homogeneous::getSigma() const {
    return sigma;
}

float& Homogeneous::getDensity() {
    return density;
}

const float& Homogeneous::getDensity() const {
    return density;
}

