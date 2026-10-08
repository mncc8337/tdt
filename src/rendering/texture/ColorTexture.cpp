#include "ColorTexture.h"
#include "HitInfo.h"

ColorTexture::ColorTexture(Color color): color(color) {}

Color ColorTexture::get(const HitInfo& rec) const {
    return color;
}


Color& ColorTexture::getColor() {
    return color;
}
