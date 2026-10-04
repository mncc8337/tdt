#pragma once

#include "Vec3.h"
#include "Mat3x3.h"

struct Transform {
    Vec3 scale = Vec3(1);
    Mat3x3 rotation;
    Vec3 translation = Vec3(0);

    Vec3 point_apply(const Vec3& v) const;
    Vec3 point_apply_inverse(const Vec3& v) const;

    Vec3 dir_apply(const Vec3& v) const;
    Vec3 dir_apply_inverse(const Vec3& v) const;

    Vec3 normal_apply(const Vec3& v) const;

    Transform& rotate(const Vec3 angles);
    Transform& move(const Vec3 pos);
    Transform& size(const Vec3 s);
};
