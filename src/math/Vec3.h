#pragma once

#include <cstdint>

struct Vec3 {
    Vec3();
    Vec3(float x, float y, float z);

    union {
        struct {
            float x, y, z;
        };
        float axis[3];
    };

    Vec3 operator -() const;

    Vec3 operator +(const Vec3& v) const;
    const Vec3& operator +=(const Vec3& v);

    Vec3 operator -(const Vec3& v) const;
    const Vec3& operator -=(const Vec3& v);

    Vec3 operator *(const float t) const;
    const Vec3& operator *=(const float x);
    friend Vec3 operator *(const float t, const Vec3& v);

    Vec3 operator *(const Vec3& v) const;
    Vec3 operator *=(const Vec3& v);

    Vec3 operator /(const float t) const;
    const Vec3& operator /=(const float t);


    float dot(const Vec3& other) const;
    Vec3 cross(const Vec3& other) const;

    float length_squared() const;
    float length() const;

    Vec3 normalized() const;
    Vec3 normalize();

    Vec3 lerp(const Vec3& v, const float t) const;

    Vec3 reflection(const Vec3& n) const;
    Vec3 refraction(const Vec3& n, const float etai_over_etat);

    std::uint32_t toABGR() const;
};

using Color = Vec3;
