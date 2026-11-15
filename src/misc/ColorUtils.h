#pragma once

#include <cstdint>
#include <vector>

#include "math/Vec3.h"

Color& gammaCorrect(Color& color, float gamma);

enum Tonemap {
    Reinhard,
    Exposure,
};

// resolve an accumulated HDR buffer into display pixels
void resolvePixels(
    const std::vector<Color>& buffer,
    std::vector<std::uint32_t>& pixels,
    unsigned pass,
    float exposure,
    float gamma,
    Tonemap tonemap
);

// TODO: turn these into classes
Color& exposureTonemap(Color& color, float exposure);
Color& reihardTonemap(Color& color);
