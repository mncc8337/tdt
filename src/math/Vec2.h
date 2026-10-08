#pragma once

#include "Vec3.h"

struct Vec2 {
    Vec2(float t = 0);
    Vec2(float x, float y);
    Vec2(const Vec3& v);

    union {
        struct {
            float x, y;
        };
        float axis[2];
    };

    float& operator [](unsigned axis);
    const float& operator [](unsigned axis) const;

    Vec2 operator -() const;

    Vec2 operator +(const Vec2& v) const;
    const Vec2& operator +=(const Vec2& v);

    Vec2 operator -(const Vec2& v) const;
    const Vec2& operator -=(const Vec2& v);

    Vec2 operator *(const float t) const;
    const Vec2& operator *=(const float x);
    friend Vec2 operator *(const float t, const Vec2& v);

    Vec2 operator *(const Vec2& v) const;
    Vec2 operator *=(const Vec2& v);

    Vec2 operator /(const float t) const;
    const Vec2& operator /=(const float t);

    float dot(const Vec2& other) const;

    float length_squared() const;
    float length() const;

    Vec2 normalized() const;
    Vec2 normalize();

    Vec2 lerp(const Vec2& v, const float t) const;
};
