#pragma once

#include "math/Vec3.h"

class Hittable;

struct HitInfo {
    Vec3 hit_point, normal;
    double distance;
    Hittable* object;
    bool front_face;

    HitInfo() : hit_point(), distance(0), normal(), object(nullptr), front_face(false) {}
};
