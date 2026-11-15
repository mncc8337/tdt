#pragma once

#include "Medium.h"

class Homogeneous: public Medium {
private:
    Color sigma;
    float density;

    // sigma * density, cached because transmit() runs once per ray segment
    Color eff_sigma;
    void refreshEffSigma();

public:
    Homogeneous(const std::string name, const float ior, const Color sigma, const float density);

    const Color& getSigma() const;
    void setSigma(const Color& sigma);

    float getDensity() const;
    void setDensity(float density);

    Color transmit(const float distance) const override;
};
