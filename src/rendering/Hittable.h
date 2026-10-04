#pragma once

#include "math/Ray.h"
#include "Material.h"
#include "HitInfo.h"
#include "math/AABB.h"

class Hittable{
protected:
    // Mat3x3 transform;
    Material* material;

public:
    Hittable(Material* material);
    virtual ~Hittable();
    virtual HitInfo hit(const Ray& ray) const = 0;
    virtual AABB getAABB() const = 0;
};
