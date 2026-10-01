#pragma once

#include "math/Ray.h"
#include "HitInfo.h"

class Hittable{
public:
    virtual ~Hittable();

    virtual HitInfo hit(const Ray& ray) const = 0;
};
