#include "Transform.h"
#include "misc/floatcmp.h"

Vec3 Transform::dir_apply(const Vec3& v) const {
    return rotation * (v * scale);
}

Vec3 Transform::dir_apply_inverse(const Vec3& v) const {
    return (rotation.transpose() * v) / scale;
}

Vec3 Transform::point_apply(const Vec3& v) const {
    return translation + dir_apply(v);
}

Vec3 Transform::point_apply_inverse(const Vec3& v) const {
    return dir_apply_inverse(v - translation);
}

Vec3 Transform::normal_apply(const Vec3& n) const {
    if(fequal(scale.x, scale.y) and fequal(scale.y, scale.z)) {
        return (rotation * n) / scale.x;
    }
    return (rotation * (n / scale)).normalized();
}

Transform& Transform::rotate(const Vec3 angles) {
    rotation = Mat3x3::rotateX(angles.x) *
             Mat3x3::rotateY(angles.y) *
             Mat3x3::rotateZ(angles.z);
    return *this;
}

Transform& Transform::move(const Vec3 pos) {
    translation = pos;
    return *this;
}

Transform& Transform::size(const Vec3 s) {
    scale = s;
    return *this;
}
