#pragma once

#include "math/Ray.h"
#include "HitInfo.h"
#include "math/AABB.h"
#include "math/Transform.h"
#include "misc/NamedElement.h"
#include <memory>

struct SurfaceSample {
    Vec3 sample;
    Vec3 sample_normal;
    float area_pdf = 0.0f;
};

class Hittable: public NamedElement {
protected:
    Transform transform;
    Material* material;

    AABB local_aabb;
    AABB global_aabb;

    virtual AABB calculateLocalAABB() const = 0;
    AABB calculateGlobalAABB() const;
    void refreshAABB();

public:
    Hittable(const std::string name, Material* material);
    virtual ~Hittable();
    virtual HitInfo hit(const Ray& ray) const = 0;
    virtual Vec2 getUV(const HitInfo& rec) const = 0;
    virtual bool samplePoint(const Vec3& viewpoint, SurfaceSample& sample) const = 0;
    virtual AABB getAABB() const;

    const Transform& getTransform() const;
    void setTransform(Transform t);

    Material* getMaterial() const;
    void setMaterial(Material* material);
};

// for the BVHNode template
AABB getNodeAABB(const std::unique_ptr<Hittable>& obj);
