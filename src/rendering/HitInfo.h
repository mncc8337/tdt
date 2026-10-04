#pragma once

#include "math/Vec3.h"
#include "math/Vec2.h"

#define DID_HIT(rec) ((rec).distance > 0.0)

class Material;

struct HitInfo {
    float distance = -1;
    Vec3 hit_point;
    Vec3 normal;
    const Material* material = nullptr;
    bool front_face;
    Vec2 uv;
};
