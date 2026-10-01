#pragma once

#include "math/Vec3.h"

class Hittable;

struct HitInfo {
    float distance = -1.0; // neg value == no hit
    Vec3 hit_point;
    Vec3 normal;
    const Hittable* object;
    bool front_face;

    HitInfo() : hit_point(), distance(0), normal(), object(nullptr), front_face(false) {}
};
