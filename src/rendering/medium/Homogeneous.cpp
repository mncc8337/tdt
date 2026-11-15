#include "medium/Homogeneous.h"
#include <cmath>

Homogeneous::Homogeneous(
    const std::string name,
    const float ior,
    const Color sigma,
    const float density
):
    Medium(name, ior),
    sigma(sigma),
    density(density),
    eff_sigma(sigma * density) {}

void Homogeneous::refreshEffSigma() {
    eff_sigma = sigma * density;
}

const Color& Homogeneous::getSigma() const {
    return sigma;
}

void Homogeneous::setSigma(const Color& sigma) {
    this->sigma = sigma;
    refreshEffSigma();
}

float Homogeneous::getDensity() const {
    return density;
}

void Homogeneous::setDensity(const float density) {
    this->density = density;
    refreshEffSigma();
}

Color Homogeneous::transmit(const float distance) const {
    // fully transparent media (the common case) skip the exp() calls
    if(eff_sigma.x == 0.0f && eff_sigma.y == 0.0f && eff_sigma.z == 0.0f)
        return Color(1);

    return Color(
        std::exp(-eff_sigma.x * distance),
        std::exp(-eff_sigma.y * distance),
        std::exp(-eff_sigma.z * distance)
    );
}

