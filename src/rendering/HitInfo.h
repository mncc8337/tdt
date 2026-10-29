#pragma once

#include "math/Vec3.h"
#include "math/Vec2.h"

class Material;

struct HitInfo {
    float distance = -1;
    Vec3 hit_point;
    Vec3 normal;
    const Material* material = nullptr;
    bool front_face;
    Vec2 uv;

    bool didHit() const;
};
