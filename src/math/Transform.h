#pragma once

#include "Vec3.h"
#include "Mat3x3.h"

struct Transform {
private:
    Vec3 size = Vec3(1);
    Mat3x3 rotation;
    Vec3 rotation_angles = Vec3(0);
    Vec3 translation = Vec3(0);

    // cache inverse of rotation and scale
    Mat3x3 inverse_rotation;
    Vec3 inverse_scale = Vec3(1);
    void refreshInverse();

public:
    Vec3 getTranslation() const;
    Vec3 getRotation() const;
    Vec3 getSize() const;

    Vec3 pointApply(const Vec3& v) const;
    Vec3 pointApplyInverse(const Vec3& v) const;

    Vec3 dirApply(const Vec3& v) const;
    Vec3 dirApplyInverse(const Vec3& v) const;

    Vec3 normalApply(const Vec3& v) const;

    Transform& rotate(const Vec3 angles);
    Transform& move(const Vec3 pos);
    Transform& scale(const Vec3 s);
};
