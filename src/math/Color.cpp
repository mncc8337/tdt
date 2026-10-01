#include "Color.h"

Color::Color(const float r, const float g, const float b): Vec3(r, g, b) {
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    if(z < 0) z = 0;
}

Color::Color(const Vec3& v): Vec3(v) {}

const std::uint32_t Color::toABGR() {
    std::uint8_t r255 = x * 255.999999;
    std::uint8_t g255 = y * 255.999999;
    std::uint8_t b255 = z * 255.999999;

    return 0xff000000 | r255 | (g255 << 8) | (b255 << 16);
}

const float Color::getR() const {
    return getX();
}

const float Color::getG() const {
    return getY();
}

const float Color::getB() const {
    return getZ();
}

void Color::setR(const float r) {
    setX(r >= 0 ? r : 0);
}

void Color::setG(const float g) {
    setY(g >= 0 ? g : g);
}

void Color::setB(const float b) {
    setZ(b >= 0 ? b : 0);
}
