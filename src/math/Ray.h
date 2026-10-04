#pragma once

#include "Vec3.h"

struct Ray {
    Ray(Vec3 origin, Vec3 direction);

    Vec3 origin;
    Vec3 direction;

    Vec3 point(const float distance) const;
};
