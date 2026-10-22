#pragma once

#include <memory>
#include <algorithm>
#include <vector>
#include <functional>
#include "misc/floatcmp.h"
#include "Hittable.h"
#include "math/AABB.h"
#include "math/RawTriangle.h"

#define PRIM_PER_LEAF 4

inline AABB getNodeAABB(const std::unique_ptr<Hittable>& obj) {
    return obj->getAABB();
}

inline AABB getNodeAABB(const RawTriangle& tri) {
    AABB ret = AABB({tri.v0, tri.v1, tri.v2});
    ret.minp -= Vec3(EPSILON);
    ret.maxp += Vec3(EPSILON);
    return ret;
}

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

struct BVHNode {
    AABB aabb;
    BVHNode* left = nullptr;
    BVHNode* right = nullptr;
    bool leaf_node = false;
    int start_index = 0;
    int num_objects = 0;
    int axis = 0;

    template <typename ObjectType>
    BVHNode(std::vector<ObjectType>& src_objects, size_t start, size_t end) {
        size_t object_span = end - start;

        AABB span_box = getNodeAABB(src_objects[start]);
        for(size_t i = start + 1; i < end; i++) {
            span_box.extend(getNodeAABB(src_objects[i]));
        }

        // find the longest axis
        float max_length = span_box.maxp[0] - span_box.minp[0];
        axis = 0;
        for(int a = 1; a < 3; a++) {
            float len = span_box.maxp[a] - span_box.minp[a];
            if(len > max_length) {
                max_length = len;
                axis = a;
            }
        }

        auto comparator = [this](const auto& a, const auto& b) {
            return getNodeAABB(a).minp[axis] < getNodeAABB(b).minp[axis];
        };

        if(object_span <= PRIM_PER_LEAF) {
            leaf_node = true;
            start_index = start;
            num_objects = object_span;
            aabb = span_box;
        } else {
            leaf_node = false;
            num_objects = 0;

            std::sort(src_objects.begin() + start, src_objects.begin() + end, comparator);

            size_t mid = start + object_span / 2;

            left = new BVHNode(src_objects, start, mid);
            right = new BVHNode(src_objects, mid, end);

            aabb = left->aabb + right->aabb;
        }
    }

    ~BVHNode() {
        if(!leaf_node) {
            delete left;
            delete right;
        }
    }

    static int flatten(
        std::vector<LinearBVHNode>& flat_bvh,
        BVHNode* node,
        int& offset
    );

    static HitInfo traverse(
        const std::vector<LinearBVHNode>& flat_bvh,
        const Ray& ray,
        std::function<void(const int, HitInfo&)> hitFunc
    );

    template <typename ObjectType>
    static void build(
        std::vector<LinearBVHNode>& flat_bvh,
        std::vector<ObjectType>& src_objects,
        size_t object_count
    ) {
        BVHNode* temp_root = new BVHNode(src_objects, 0, object_count);
        flat_bvh.resize(object_count * 2);

        int offset = 0;
        flatten(flat_bvh, temp_root, offset);
        flat_bvh.resize(offset);

        delete temp_root;
    }
};
