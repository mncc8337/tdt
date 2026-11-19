#pragma once

#include "math/Vec3.h"
#include "math/Vec2.h"

class Material;
class Hittable;

struct HitInfo {
    float distance = -1;
    Vec3 hit_point;
    Vec3 normal;
    bool front_face;
    const Material* material = nullptr;
    const Hittable* hittable = nullptr;
    Vec2 tri_uv = Vec2(-1.0f, -1.0f);

    bool didHit() const {
        return distance > 0.0f;
    }
};
