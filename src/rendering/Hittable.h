#pragma once

#include "math/Ray.h"
#include "Material.h"
#include "HitInfo.h"
#include "math/AABB.h"
#include "math/Transform.h"

class Hittable{
protected:
    Transform transform;
    Material* material;

    virtual AABB getLocalAABB() const = 0;

public:
    Hittable(Material* material);
    virtual ~Hittable();
    virtual HitInfo hit(const Ray& ray) const = 0;
    virtual AABB getAABB() const;

    Transform& getTransform();
};
