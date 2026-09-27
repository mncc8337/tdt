#pragma once

#include <cstdint>

class Color {
private:
    double r;
    double g;
    double b;

public:
    Color(double r, double g, double b);

    const double getR();
    const double getG();
    const double getB();

    void setR(const double x);
    void setG(const double x);
    void setB(const double x);

    const std::uint32_t toABGR();

    Color operator +(Color cl);
    Color operator +=(Color cl);

    Color operator -(Color cl);
    Color operator -=(Color cl);

    Color operator *(double x);
    Color operator *=(double x);
};
