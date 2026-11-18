#include "ColorTexture.h"
#include "HitInfo.h"

ColorTexture::ColorTexture(const std::string name, Color color):
    Texture(name),
    color(color) {}

Color ColorTexture::get(const HitInfo& rec) const {
    return color;
}


const Color& ColorTexture::getColor() {
    return color;
}

void ColorTexture::setColor(const Color c) {
    color = c;
}
