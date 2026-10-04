#pragma once

#include "Vec3.h"

struct Mat3x3 {
    float m[3][3] = {0};

    Mat3x3();

    Vec3 operator *(const Vec3& v) const;
    Mat3x3 operator *(const Mat3x3& right) const;

    Mat3x3 transpose() const;

    static Mat3x3 rotateX(const float angle);
    static Mat3x3 rotateY(const float angle);
    static Mat3x3 rotateZ(const float angle);
};
