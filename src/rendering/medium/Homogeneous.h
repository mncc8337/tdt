#pragma once

#include "Medium.h"

class Homogeneous: public Medium {
private:
    Color sigma;
    float density;

public:
    Homogeneous(const float ior, const Color sigma, const float density);

    Color& getSigma();
    const Color& getSigma() const;

    float& getDensity();
    const float& getDensity() const;

    Color transmit(const float distance) const override;
};
