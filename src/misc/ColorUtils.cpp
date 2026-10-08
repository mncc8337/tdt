#include "ColorUtils.h"
#include <cmath>

Color& gammaCorrect(Color& color, float gamma) {
    color.x = std::pow(color.x, 1.0f / gamma);
    color.y = std::pow(color.y, 1.0f / gamma);
    color.z = std::pow(color.z, 1.0f / gamma);
    return color;
}

Color& exposureTonemap(Color& color, float exposure) {
    // result is mathematically < 1 for finite input; clamp so negative raw
    // values (and rounding above 1) can never escape this range.
    auto tm = [exposure](float v) {
        if (v < 0.0f || std::isnan(v)) v = 0.0f;
        return std::fmin(1.0f - std::exp(-v * exposure), 1.0f);
    };
    color.x = tm(color.x);
    color.y = tm(color.y);
    color.z = tm(color.z);
    return color;
}

Color& reihardTonemap(Color& color) {
    color /= color + Color(1);
    return color;
}
