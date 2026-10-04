#include "Vec3.h"

#include <cmath>

Vec3::Vec3() : x(0), y(0), z(0) {}
Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

float Vec3::getX() const {
    return x;
}

float Vec3::getY() const {
    return y;
}

float Vec3::getZ() const {
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

Vec3 Vec3::operator *=(const Vec3& v) {
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
    return (1.0 - t) * (*this) + v * t;
}

Vec3 Vec3::reflection(const Vec3& n) const {
    return (*this) - 2 * (*this).dot(n) * n;
}

Vec3 Vec3::refraction(const Vec3& n, const float etai_over_etat) {
    float cos_theta = std::fmin(-(*this).dot(n), 1.0);
    Vec3 r_out_perp =  etai_over_etat * ((*this) + cos_theta*n);
    Vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}
