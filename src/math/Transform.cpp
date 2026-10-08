#include "Transform.h"
#include "misc/floatcmp.h"

Vec3 Transform::dirApply(const Vec3& v) const {
    return rotation * (v * scale);
}

Vec3 Transform::dirApplyInverse(const Vec3& v) const {
    return (rotation.transpose() * v) / scale;
}

Vec3 Transform::pointApply(const Vec3& v) const {
    return translation + dirApply(v);
}

Vec3 Transform::pointApplyInverse(const Vec3& v) const {
    return dirApplyInverse(v - translation);
}

Vec3 Transform::normalApply(const Vec3& n) const {
    if(fequal(scale.x, scale.y) and fequal(scale.y, scale.z)) {
        return (rotation * n) / scale.x;
    }
    return (rotation * (n / scale)).normalized();
}

Transform& Transform::rotate(const Vec3 angles) {
    rotation_angles = angles;
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
