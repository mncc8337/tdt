#include "Color.h"
#include <cmath>

Color::Color(double r, double g, double b): r(r), g(g), b(b) {
    if(r < 0) r = 0;
    if(g < 0) g = 0;
    if(b < 0) b = 0;
}

const std::uint32_t Color::toABGR() {
    std::uint8_t r255 = r * 255.999999;
    std::uint8_t g255 = g * 255.999999;
    std::uint8_t b255 = b * 255.999999;

    return 0xff000000 | r255 | (g255 << 8) | (b255 << 16);
}

const double Color::getR() {
    return r;
}

const double Color::getG() {
    return g;
}

const double Color::getB() {
    return b;
}

void Color::setR(const double x) {
    r = x;
}

void Color::setG(const double x) {
    g = x;
}

void Color::setB(const double x) {
    b = x;
}

Color Color::operator +(Color cl) {
    return Color(r + cl.getR(), g + cl.getG(), b + cl.getB());
}

Color Color::operator +=(Color cl) {
    r += cl.getR();
    g += cl.getG();
    b += cl.getB();

    return *this;
}

Color Color::operator -(Color cl) {
    return Color(r - cl.getR(), g - cl.getG(), b - cl.getB());
}

Color Color::operator -=(Color cl) {
    r -= cl.getR();
    g -= cl.getG();
    b -= cl.getB();

    if(r < 0) r = 0;
    if(g < 0) g = 0;
    if(b < 0) b = 0;

    return *this;
}

Color Color::operator *(double x) {
    x = std::abs(x);
    return Color(r * x, g * x, b * x);
}

Color Color::operator *=(double x) {
    x = std::abs(x);
    r *= x;
    g *= x;
    b *= x;
    return *this;
}
