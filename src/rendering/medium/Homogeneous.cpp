#include "medium/Homogeneous.h"
#include "floatcmp.h"
#include "misc/RNG.h"
#include <algorithm>
#include <cmath>

Homogeneous::Homogeneous(
    unsigned id,
    const std::string name,
    const float ior,
    const Color sigma_a,
    const Color sigma_s,
    const float density
):
    Medium(id, name, ior),
    sigma_a(sigma_a),
    sigma_s(sigma_s),
    density(density) {
    refreshEffSigma();
}

void Homogeneous::refreshEffSigma() {
    eff_sigma_a = sigma_a * density;
    eff_sigma_s = sigma_s * density;
    eff_sigma_t = eff_sigma_a + eff_sigma_s;
    majorant = std::max({
        eff_sigma_t.x,
        eff_sigma_t.y,
        eff_sigma_t.z
    });
    scattering = (
        !fequal0(eff_sigma_s.x) ||
        !fequal0(eff_sigma_s.y) ||
        !fequal0(eff_sigma_s.z)
    );
}

const Color& Homogeneous::getSigmaA() const {
    return sigma_a;
}

void Homogeneous::setSigmaA(const Color& sigma_a) {
    this->sigma_a = sigma_a;
    refreshEffSigma();
}

const Color& Homogeneous::getSigmaS() const {
    return sigma_s;
}

void Homogeneous::setSigmaS(const Color& sigma_s) {
    this->sigma_s = sigma_s;
    refreshEffSigma();
}

float Homogeneous::getDensity() const {
    return density;
}

void Homogeneous::setDensity(const float density) {
    this->density = density;
    refreshEffSigma();
}

Color Homogeneous::transmittance(const float distance) const {
    return Color(
        std::exp(-eff_sigma_t.x * distance),
        std::exp(-eff_sigma_t.y * distance),
        std::exp(-eff_sigma_t.z * distance)
    );
}

float Homogeneous::transmit(Ray& ray, Color& throughput, const float& distance) const {
    // fully transparent media (the common case) skip the exp() calls
    if(fequal0(majorant))
        return distance;

    // non-scattering media are deterministic: pure Beer-Lambert attenuation
    if(!scattering) {
        throughput *= transmittance(distance);
        return distance;
    }

    // sample free-flight distance, clamped to the segment length.
    // uniform() returns [0, 1): it can produce 0, which would make
    // log(0) = -inf, and cannot produce 1. So 1 - uniform() is in (0, 1],
    // which keeps the log argument positive without truncating the
    // exponential tail and costs the same as using uniform() directly.
    float t = std::min(
        -std::log(1.0f - RNG::uniform()) / majorant,
        distance
    );

    Color attenuation(
        std::exp(-(eff_sigma_t.x - majorant) * t),
        std::exp(-(eff_sigma_t.y - majorant) * t),
        std::exp(-(eff_sigma_t.z - majorant) * t)
    );

    if(t < distance) {
        // collision: weight by the scattering coefficient
        // absorption is the part of sigma_t that is not scattered
        attenuation *= eff_sigma_s / majorant;
        ray.origin += ray.direction * t;
        ray.direction = RNG::direction();
    }

    throughput *= attenuation;

    return t;
}
