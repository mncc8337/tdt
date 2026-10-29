#pragma once

#include <memory>
#include <vector>
#include <functional>
#include "Hittable.h"
#include "math/AABB.h"
#include "math/RawTriangle.h"

#define PRIM_PER_LEAF 4

AABB getNodeAABB(const std::unique_ptr<Hittable>& obj);
AABB getNodeAABB(const RawTriangle& tri);

template <typename ObjectType>
struct BVHNode {
    AABB aabb;
    std::unique_ptr<BVHNode> left = nullptr;
    std::unique_ptr<BVHNode> right = nullptr;
    int start_index = 0;
    int num_objects = 0;

    bool isLeaf() const;

    BVHNode(std::vector<ObjectType>& src_objects, size_t start, size_t end);

    HitInfo traverse(
        const Ray& ray,
        std::function<void(const int, HitInfo&)> hitFunc
    ) const;
};
