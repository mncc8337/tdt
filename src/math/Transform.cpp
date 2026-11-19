#include "Transform.h"
#include "misc/floatcmp.h"

void Transform::refreshInverse() {
    inverse_rotation = rotation.transpose();
    inverse_scale = size.inverse();
}

Vec3 Transform::getTranslation() const {
    return translation;
}

Vec3 Transform::getRotation() const {
    return rotation_angles;
}

Vec3 Transform::getSize() const {
    return size;
}

Vec3 Transform::dirApply(const Vec3& v) const {
    return rotation * (v * size);
}

Vec3 Transform::dirApplyInverse(const Vec3& v) const {
    return (inverse_rotation * v) / size;
}

Vec3 Transform::pointApply(const Vec3& v) const {
    return translation + dirApply(v);
}

Vec3 Transform::pointApplyInverse(const Vec3& v) const {
    return dirApplyInverse(v - translation);
}

Vec3 Transform::normalApply(const Vec3& n) const {
    if(fequal(size.x, size.y) and fequal(size.y, size.z)) {
        return rotation * n;
    }
    return (rotation * (n * inverse_scale)).normalized();
}

const Mat3x3& Transform::getRotationMatrix() const {
    return rotation;
}
const Mat3x3& Transform::getInverseRotationMatrix() const {
    return inverse_rotation;
}

Transform& Transform::rotate(const Vec3 angles) {
    rotation_angles = angles;
    rotation = Mat3x3::rotateX(angles.x) *
             Mat3x3::rotateY(angles.y) *
             Mat3x3::rotateZ(angles.z);
    refreshInverse();
    return *this;
}

Transform& Transform::move(const Vec3 pos) {
    translation = pos;
    return *this;
}

Transform& Transform::scale(const Vec3 s) {
    size = s;
    refreshInverse();
    return *this;
}
