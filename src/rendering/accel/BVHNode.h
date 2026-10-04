#pragma once

#include <memory>
#include <algorithm>
#include "Hittable.h"
#include "math/AABB.h"
#include "math/RawTriangle.h"

struct BVHNode {
    AABB aabb;
    BVHNode* left = nullptr;
    BVHNode* right = nullptr;
    bool leaf_node = false;
    int start_index = 0;
    int num_objects = 0;
    int axis = 0;

    template <typename T>
    BVHNode(std::vector<T>& src_objects, size_t start, size_t end) {
        size_t object_span = end - start;

        AABB span_box = get_node_aabb(src_objects[start]);
        for(size_t i = start + 1; i < end; i++) {
            span_box.extend(get_node_aabb(src_objects[i]));
        }

        // find the longest axis
        float max_length = span_box.maxp[0] - span_box.minp[0];
        axis = 0;
        for (int a = 1; a < 3; a++) {
            float len = span_box.maxp[a] - span_box.minp[a];
            if (len > max_length) {
                max_length = len;
                axis = a;
            }
        }

        auto comparator = [this](const auto& a, const auto& b) {
            return get_node_aabb(a).minp[axis] < get_node_aabb(b).minp[axis];
        };

        if(object_span <= 2) { 
            leaf_node = true;
            start_index = start;
            num_objects = object_span;
            aabb = span_box;
        } else {
            leaf_node = false;
            num_objects = 0;

            std::sort(src_objects.begin() + start, src_objects.begin() + end, comparator);

            auto mid = start + object_span / 2;

            left = new BVHNode(src_objects, start, mid);
            right = new BVHNode(src_objects, mid, end);

            aabb = left->aabb + right->aabb;
        }
    }

    ~BVHNode() {
        if (!leaf_node) {
            delete left;
            delete right;
        }
    }
};

struct LinearBVHNode {
    AABB aabb;
    
    union {
        int primitives_offset; // leaf node's first obj idx
        int right_offset; // branch node's right node idx
    };
    
    uint16_t num_objects; // equals 0 if is a branch
    uint8_t axis;
    uint8_t pad[1];
};

inline AABB get_node_aabb(const std::unique_ptr<Hittable>& obj) {
    return obj->getAABB();
}

inline AABB get_node_aabb(const RawTriangle& tri) {
    return AABB({tri.v0, tri.v1, tri.v2}); 
}
