#pragma once
#include "BVHNode.h"
#include <algorithm>

template <typename ObjectType>
BVHNode::BVHNode(std::vector<ObjectType>& src_objects, size_t start, size_t end) {
    if(start >= end) {
        num_objects = 0;
        return;
    }

    size_t object_span = end - start;
    AABB span_box = getNodeAABB(src_objects[start]);
    for(size_t i = start + 1; i < end; i++) {
        span_box.extend(getNodeAABB(src_objects[i]));
    }

    if(object_span <= PRIM_PER_LEAF) {
        left = nullptr;
        right = nullptr;

        start_index = start;
        num_objects = object_span;
        aabb = span_box;
        return;
    }

    // find the longest axis
    int axis = 0;
    float max_length = span_box.maxp[0] - span_box.minp[0];
    for(int a = 1; a < 3; a++) {
        float len = span_box.maxp[a] - span_box.minp[a];
        if(len > max_length) {
            max_length = len;
            axis = a;
        }
    }

    auto comparator = [axis](const auto& a, const auto& b) {
        return getNodeAABB(a).minp[axis] < getNodeAABB(b).minp[axis];
    };

    num_objects = 0;
    std::sort(
        src_objects.begin() + start,
        src_objects.begin() + end,
        comparator
    );

    size_t mid = start + object_span / 2;
    left = std::make_unique<BVHNode>(src_objects, start, mid);
    right = std::make_unique<BVHNode>(src_objects, mid, end);

    aabb = span_box;
}

