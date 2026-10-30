#pragma once

#include "math/Ray.h"
#include "HitInfo.h"
#include "math/AABB.h"
#include "math/Transform.h"
#include "misc/NamedElement.h"
#include <memory>

class Hittable: public NamedElement {
protected:
    Transform transform;
    Material* material;

    AABB local_aabb;
    AABB global_aabb;

    virtual AABB calculateLocalAABB() const = 0;
    AABB calculateGlobalAABB() const;
    void recomputeAABB();

public:
    Hittable(const std::string name, Material* material);
    virtual ~Hittable();
    virtual HitInfo hit(const Ray& ray) const = 0;
    virtual AABB getAABB() const;

    const Transform& getTransform() const;
    void setTransform(const Transform t);

    Material* getMaterial() const;
};

// for the BVHNode template
AABB getNodeAABB(const std::unique_ptr<Hittable>& obj);
