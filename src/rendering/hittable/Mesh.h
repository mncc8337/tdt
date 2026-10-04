#pragma once

#include "Hittable.h"
#include "math/RawTriangle.h"
#include "accel/BVHNode.h"

class Mesh: public Hittable {
private:
    std::vector<RawTriangle> tris;
    std::vector<LinearBVHNode> flat_bvh;
    AABB aabb;

    void init_bvh();
    int flatten_bvh_tree(BVHNode* node, int& offset);

    AABB getLocalAABB() const override;

public:
    Mesh(Material* material, const std::vector<RawTriangle>& tris);
    Mesh(Material* material, std::string filename);

    HitInfo hit(const Ray& ray) const override;
};
