#pragma once

#include "math/Ray.h"
#include "Material.h"
#include "HitInfo.h"
#include "math/AABB.h"
#include "math/Transform.h"
#include <string>

class Hittable{
protected:
    Transform transform;
    Material* material;
    std::string name;

    virtual AABB getLocalAABB() const = 0;

public:
    Hittable(const std::string name, Material* material);
    virtual ~Hittable();
    virtual HitInfo hit(const Ray& ray) const = 0;
    virtual AABB getAABB() const;


    Transform& getTransform();
    const Transform& getTransform() const;

    Material* getMaterial() const;

    std::string& getName();
    const std::string& getName() const;
};
