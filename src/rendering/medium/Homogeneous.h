#pragma once

#include "Medium.h"

class Homogeneous: public Medium {
private:
    // per-unit-density coefficients
    Color sigma_a;
    Color sigma_s;
    float density;

    // sigma * density, cached because transmit() runs once per ray segment
    Color eff_sigma_a;
    Color eff_sigma_s;
    Color eff_sigma_t;
    float majorant;
    bool scattering;
    void refreshEffSigma();

public:
    Homogeneous(
        unsigned id,
        const std::string name,
        const float ior,
        const Color sigma_a,
        const Color sigma_s,
        const float density
    );

    const Color& getSigmaA() const;
    void setSigmaA(const Color& sigma_a);

    const Color& getSigmaS() const;
    void setSigmaS(const Color& sigma_s);

    float getDensity() const;
    void setDensity(float density);

    // deterministic transmittance of the effective extinction
    Color transmittance(const float distance) const;

    float transmit(Ray& ray, Color& throughput, const float& distance) const override;
};
