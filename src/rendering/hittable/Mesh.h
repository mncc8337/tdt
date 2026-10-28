#pragma once

#include "Hittable.h"
#include "math/RawTriangle.h"
#include "accel/BVHNode.h"

class Mesh: public Hittable {
private:
    std::vector<RawTriangle> tris;
    std::unique_ptr<BVHNode> bvh_root;
    AABB aabb;

    AABB getLocalAABB() const override;
    void computeAABB();

public:
    Mesh(const std::string name, Material* material, const std::vector<RawTriangle>& tris);
    Mesh(const std::string name, Material* material, std::string filename);

    HitInfo hit(const Ray& ray) const override;
};
