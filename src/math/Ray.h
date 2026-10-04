#pragma once

#include "Vec3.h"

struct Ray {
    Ray(Vec3 origin = Vec3(0), Vec3 direction = Vec3(0, 0, -1));

    Vec3 origin;
    Vec3 direction;

    Vec3 point(const float distance) const;
};
