#include "Vec2.h"
#include <cmath>

Vec2::Vec2(float t) : x(t), y(t) {}
Vec2::Vec2(float x, float y) : x(x), y(y) {}

Vec2::Vec2(const Vec3& v):
    x(v.x), y(v.y) {}


float& Vec2::operator [](const unsigned a) {
    return axis[a];
}

const float& Vec2::operator [](const unsigned a) const {
    return axis[a];
}

Vec2 Vec2::operator -() const {
    return Vec2(-x, -y);
}

Vec2 Vec2::operator +(const Vec2& v) const {
    return Vec2(x + v.x, y + v.y);
}

const Vec2& Vec2::operator +=(const Vec2& v) {
    *this = *this + v;
    return *this;
}

Vec2 Vec2::operator -(const Vec2& v) const {
    return *this + (-v);
}

const Vec2& Vec2::operator -=(const Vec2& v) {
    *this = *this + (-v);
    return *this;
}

Vec2 Vec2::operator *(const float t) const {
    return Vec2(x * t, y * t);
}

const Vec2& Vec2::operator *=(const float t) {
    *this = *this * t;
    return *this;
}

Vec2 Vec2::operator *(const Vec2& v) const {
    return Vec2(this->x * v.x, this->y * v.y);
}

Vec2 Vec2::operator *=(const Vec2& v) {
    *this = *this * v;
    return *this;
}

Vec2 operator *(const float t, const Vec2& v) {
    return v * t;
}

Vec2 Vec2::operator /(const float t) const {
    return Vec2(x / t, y / t);
}

const Vec2& Vec2::operator /=(const float t) {
    *this = *this / t;
    return *this;
}

float Vec2::dot(const Vec2& other) const {
    return x * other.x + y * other.y;
}

float Vec2::length_squared() const {
    return this->dot(*this);
}

float Vec2::length() const {
    return std::sqrt(length_squared());
}

Vec2 Vec2::normalized() const {
    float len = length();
    return *this / len;
}

Vec2 Vec2::normalize() {
    *this = *this / length();
    return *this;
}

Vec2 Vec2::lerp(const Vec2& v, const float t) const {
    return (*this) * (1.0f - t) + v * t;
}
