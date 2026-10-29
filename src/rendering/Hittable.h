#pragma once

#include "math/Ray.h"
#include "HitInfo.h"
#include "math/AABB.h"
#include "math/Transform.h"
#include "misc/NamedElement.h"

class Hittable: public NamedElement {
protected:
    Transform transform;
    Material* material;

    virtual AABB getLocalAABB() const = 0;

public:
    Hittable(const std::string name, Material* material);
    virtual ~Hittable();
    virtual HitInfo hit(const Ray& ray) const = 0;
    virtual AABB getAABB() const;

    const Transform& getTransform() const;
    // FIXME: this func allows user to set the object transform without the scene knowing
    void setTransform(const Transform t);

    Material* getMaterial() const;
};
