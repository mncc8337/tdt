#pragma once

#include "math/Ray.h"
#include "Material.h"
#include "HitInfo.h"

class Hittable{
protected:
    // Mat3x3 transform;
    Material* material;

public:
    Hittable(Material* material);
    virtual HitInfo hit(const Ray& ray) const = 0;
};
