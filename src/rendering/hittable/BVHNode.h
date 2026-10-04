#pragma once

#include <memory>
#include "Hittable.h"
#include "math/AABB.h"

class BVHNode : public Hittable {
private:
    AABB aabb;

    Hittable* left;
    Hittable* right;

    std::unique_ptr<BVHNode> branch_left;
    std::unique_ptr<BVHNode> branch_right;

public:
    BVHNode(std::vector<Hittable*>& src_objects, size_t start, size_t end);

    HitInfo hit(const Ray& ray) const override;
    AABB getAABB() const override;
};
