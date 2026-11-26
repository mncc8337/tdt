#include "SolidTexture.h"
#include "HitInfo.h"

ColorTexture::ColorTexture(unsigned id, const std::string name, Color color):
    Texture(id, name),
    color(color) {}

Color ColorTexture::get(const HitInfo& rec) const {
    return color;
}


const Color& ColorTexture::getColor() const {
    return color;
}

void ColorTexture::setColor(const Color c) {
    color = c;
}
