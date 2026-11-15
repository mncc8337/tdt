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
        if(v < 0.0f || std::isnan(v)) v = 0.0f;
        return std::fmin(1.0f - std::exp(-v * exposure), 1.0f);
    };
    color.x = tm(color.x);
    color.y = tm(color.y);
    color.z = tm(color.z);
    return color;
}

Color& reihardTonemap(Color& color) {
    color.x = std::fmax(color.x, 0.0f);
    color.y = std::fmax(color.y, 0.0f);
    color.z = std::fmax(color.z, 0.0f);
    color /= color + Color(1);
    return color;
}

void resolvePixels(
    const std::vector<Color>& buffer,
    std::vector<std::uint32_t>& pixels,
    unsigned pass,
    float exposure,
    float gamma,
    Tonemap tonemap
) {
    pixels.resize(buffer.size());

    const float inv_pass = pass > 0 ? 1.0f / float(pass) : 0.0f;

    for(size_t i = 0; i < buffer.size(); i++) {
        Color display = buffer[i] * inv_pass;

        switch(tonemap) {
            case Tonemap::Reinhard:
                display *= exposure;
                reihardTonemap(display);
                break;
            case Tonemap::Exposure:
                exposureTonemap(display, exposure);
                break;
        }

        gammaCorrect(display, gamma);
        pixels[i] = display.toABGR();
    }
}
