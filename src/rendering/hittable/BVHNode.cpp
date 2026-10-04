#include "BVHNode.h"
#include "misc/floatcmp.h"
#include <algorithm>

#include "BVHNode.h"
#include <algorithm>

BVHNode::BVHNode(std::vector<Hittable*>& src_objects, size_t start, size_t end) :
    Hittable(nullptr), aabb(Vec3()) {
    size_t object_span = end - start;

    AABB span_box = src_objects[start]->getAABB();
    for(size_t i = start + 1; i < end; i++) {
        span_box.extend(src_objects[i]->getAABB());
    }

    // find the largest axis
    int axis = 0;
    float max_length = span_box.maxp[0] - span_box.minp[0];
    for(int a = 1; a < 3; a++) {
        float len = span_box.maxp[a] - span_box.minp[a];
        if (len > max_length) {
            max_length = len;
            axis = a;
        }
    }

    auto comparator = [axis](const Hittable* a, const Hittable* b) {
        return a->getAABB().minp[axis] < b->getAABB().minp[axis];
    };

    if(object_span == 1) {
        left = right = src_objects[start];
    } else if(object_span == 2) {
        if (comparator(src_objects[start], src_objects[start + 1])) {
            left = src_objects[start];
            right = src_objects[start + 1];
        } else {
            left = src_objects[start + 1];
            right = src_objects[start];
        }
    } else {
        std::sort(src_objects.begin() + start, src_objects.begin() + end, comparator);
        auto mid = start + object_span / 2;

        branch_left = std::make_unique<BVHNode>(src_objects, start, mid);
        branch_right = std::make_unique<BVHNode>(src_objects, mid, end);

        left = branch_left.get();
        right = branch_right.get();
    }

    aabb = left->getAABB() + right->getAABB();
}

HitInfo BVHNode::hit(const Ray& ray) const {
    HitInfo rec;

    if(!aabb.hit(ray, EPSILON, FAR_DISTANCE)) { 
        return rec; 
    }

    HitInfo left_rec = left->hit(ray);
    HitInfo right_rec = right->hit(ray);

    if (DID_HIT(left_rec) && DID_HIT(right_rec)) {
        return (left_rec.distance < right_rec.distance) ? left_rec : right_rec;
    } else if (DID_HIT(left_rec)) {
        return left_rec;
    } else if (DID_HIT(right_rec)) {
        return right_rec;
    }

    return rec;
}

AABB BVHNode::getAABB() const {
    return aabb;
}
