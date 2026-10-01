#include "Vec3.h"

#include <cmath>

Vec3::Vec3() : x(0), y(0), z(0) {}
Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

const float Vec3::getX() const {
    return x;
}

const float Vec3::getY() const {
    return y;
}

const float Vec3::getZ() const {
    return x;
}

void Vec3::setX(const float x) {
    this->x = x;
}

void Vec3::setY(const float y) {
    this->y = y;
}

void Vec3::setZ(const float z) {
    this->z = z;
}

const Vec3 Vec3::operator -() const {
    return Vec3(-x, -y, -z);
}

const Vec3 Vec3::operator +(const Vec3& v) const {
    return Vec3(x + v.x, y + v.y, z + v.z);
}

const Vec3& Vec3::operator +=(const Vec3& v) {
    *this = *this + v;
    return *this;
}

const Vec3 Vec3::operator -(const Vec3& v) const {
    return *this + (-v);
}

const Vec3& Vec3::operator -=(const Vec3& v) {
    *this = *this + (-v);
    return *this;
}

const Vec3 Vec3::operator *(const float t) const {
    return Vec3(x * t, y * t, z * t);
}

const Vec3& Vec3::operator *=(const float t) {
    *this = *this * t;
    return *this;
}

const Vec3 Vec3::operator /(const float t) const {
    return Vec3(x / t, y / t, z / t);
}

const Vec3& Vec3::operator /=(const float t) {
    *this = *this / t;
    return *this;
}

const float Vec3::dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

const Vec3 Vec3::cross(const Vec3& other) const {
    return Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

const float Vec3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

const Vec3 Vec3::normalized() const {
    double len = length();
    return *this / len;
}

