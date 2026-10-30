#pragma once

#include "math/AABB.h"
#include "rendering/HitInfo.h"
#include <memory>
#include <vector>
#include <functional>

#define PRIM_PER_LEAF 4

struct BVHNode {
    AABB aabb;
    std::unique_ptr<BVHNode> left = nullptr;
    std::unique_ptr<BVHNode> right = nullptr;
    int start_index = 0;
    int num_objects = 0;

    bool isLeaf() const;

    template <typename ObjectType>
    BVHNode(std::vector<ObjectType>& src_objects, size_t start, size_t end);

    HitInfo traverse(
        const Ray& ray,
        std::function<void(const int, HitInfo&)> hitFunc
    ) const;
};

#include "BVHNode.inl"
