#pragma once

#include <cstdint>
#include "Vec3.h"

class Color: public Vec3 {
public:
    Color(const float r = 0, const float g = 0, const float b = 0);
    Color(const Vec3& v);

    const float getR() const;
    const float getG() const;
    const float getB() const;

    void setR(const float x);
    void setG(const float x);
    void setB(const float x);

    const std::uint32_t toABGR();
};
