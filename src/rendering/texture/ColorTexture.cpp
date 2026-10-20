#include "ColorTexture.h"
#include "HitInfo.h"

ColorTexture::ColorTexture(const std::string name, Color color):
    Texture(name),
    color(color) {}

Color ColorTexture::get(const HitInfo& rec) const {
    return color;
}


Color& ColorTexture::getColor() {
    return color;
}
