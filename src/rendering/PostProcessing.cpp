#include "PostProcessing.h"
#include <cmath>
#include <algorithm>

Color& clamp(Color& color) {
    color.x = std::min(color.x, 1.0f);
    color.y = std::min(color.y, 1.0f);
    color.z = std::min(color.z, 1.0f);
    return color;
}

Color& gammaCorrect(Color& color, float gamma) {
    color.x = std::pow(color.x, 1.0f / gamma);
    color.y = std::pow(color.y, 1.0f / gamma);
    color.z = std::pow(color.z, 1.0f / gamma);
    return color;
}

Color& exposureTonemap(Color& color, float exposure) {
    color.x = 1.0f - std::exp(-color.x * exposure);
    color.y = 1.0f - std::exp(-color.y * exposure);
    color.z = 1.0f - std::exp(-color.z * exposure);
    return color;
}

Color& reihardTonemap(Color& color) {
    color /= color + Color(1);
    return color;
}
