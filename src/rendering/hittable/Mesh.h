#pragma once

#include "Hittable.h"
#include "math/RawTriangle.h"
#include "dsa/BVHNode.h"

class Mesh: public Hittable {
private:
    std::vector<RawTriangle> tris;
    std::unique_ptr<BVHNode> bvh_root;

    AABB calculateLocalAABB() const override;

public:
    Mesh(const std::string name, Material* material, const std::vector<RawTriangle>& tris);
    Mesh(const std::string name, Material* material, std::string filename);

    HitInfo hit(const Ray& ray) const override;
};
