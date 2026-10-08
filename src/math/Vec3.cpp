#include "Vec3.h"

#include <cmath>

Vec3::Vec3(float t) : x(t), y(t), z(t) {}
Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

float& Vec3::operator [](const unsigned a) {
    return axis[a];
}

const float& Vec3::operator [](const unsigned a) const {
    return axis[a];
}

Vec3 Vec3::operator -() const {
    return Vec3(-x, -y, -z);
}

Vec3 Vec3::operator +(const Vec3& v) const {
    return Vec3(x + v.x, y + v.y, z + v.z);
}

const Vec3& Vec3::operator +=(const Vec3& v) {
    *this = *this + v;
    return *this;
}

Vec3 Vec3::operator -(const Vec3& v) const {
    return *this + (-v);
}

const Vec3& Vec3::operator -=(const Vec3& v) {
    *this = *this + (-v);
    return *this;
}

Vec3 Vec3::operator *(const float t) const {
    return Vec3(x * t, y * t, z * t);
}

const Vec3& Vec3::operator *=(const float t) {
    *this = *this * t;
    return *this;
}

Vec3 Vec3::operator *(const Vec3& v) const {
    return Vec3(this->x * v.x, this->y * v.y, this->z * v.z);
}

const Vec3& Vec3::operator *=(const Vec3& v) {
    *this = *this * v;
    return *this;
}

Vec3 operator *(const float t, const Vec3& v) {
    return v * t;
}

Vec3 Vec3::operator /(const float t) const {
    return Vec3(x / t, y / t, z / t);
}

const Vec3& Vec3::operator /=(const float t) {
    *this = *this / t;
    return *this;
}

Vec3 Vec3::operator /(const Vec3& v) const {
    return Vec3(
        x / v.x,
        y / v.y,
        z / v.z
    );
}

const Vec3& Vec3::operator /=(const Vec3& v) {
    *this = *this / v;
    return *this;
}

float Vec3::dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vec3 Vec3::cross(const Vec3& other) const {
    return Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

float Vec3::length_squared() const {
    return this->dot(*this);
}

float Vec3::length() const {
    return std::sqrt(length_squared());
}

Vec3 Vec3::normalized() const {
    float len = length();
    return *this / len;
}

Vec3 Vec3::normalize() {
    *this = *this / length();
    return *this;
}

Vec3 Vec3::lerp(const Vec3& v, const float t) const {
    return (*this) * (1.0f - t) + v * t;
}

Vec3 Vec3::reflection(const Vec3& n) const {
    return (*this) - n * (*this).dot(n) * 2.0f;
}

Vec3 Vec3::refraction(const Vec3& n, const float etai_over_etat) {
    float cos_theta = std::fmin(-(*this).dot(n), 1.0f);
    Vec3 r_out_perp =  ((*this) + n * cos_theta) * etai_over_etat;
    Vec3 r_out_parallel = -n * sqrt(fabs(1.0f - r_out_perp.length_squared()));
    return r_out_perp + r_out_parallel;
}

std::uint32_t Vec3::toABGR() const {
    auto channel = [](float v) {
        v = std::fmin(std::fmax(v, 0.0f), 1.0f);
        return static_cast<std::uint8_t>(v * 255.0f + 0.5f);
    };

    return 0xff000000 | channel(x) | (channel(y) << 8) | (channel(z) << 16);
}


Vec3& Vec3::clampColor() {
    this->x = std::fmin(std::fmax(this->x, 0.0f), 1.0f);
    this->y = std::fmin(std::fmax(this->y, 0.0f), 1.0f);
    this->z = std::fmin(std::fmax(this->z, 0.0f), 1.0f);
    return *this;
}
