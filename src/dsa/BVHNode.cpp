#include "BVHNode.h"
#include "misc/floatcmp.h"
#include <memory>
#include <queue>
#include <algorithm>

AABB getNodeAABB(const std::unique_ptr<Hittable>& obj) {
    return obj->getAABB();
}

AABB getNodeAABB(const RawTriangle& tri) {
    AABB ret = AABB({tri.v0, tri.v1, tri.v2});
    ret.minp -= Vec3(EPSILON);
    ret.maxp += Vec3(EPSILON);
    return ret;
}

template <typename ObjectType>
bool BVHNode<ObjectType>::isLeaf() const {
    return !left and !right;
}

template <typename ObjectType>
BVHNode<ObjectType>::BVHNode(std::vector<ObjectType>& src_objects, size_t start, size_t end) {
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

template <typename ObjectType>
HitInfo BVHNode<ObjectType>::traverse(
    const Ray& ray,
    std::function<void(const int, HitInfo&)> hitFunc
) const {
    HitInfo closest_hit;

    std::queue<const BVHNode*> node_queue;
    node_queue.push(this);

    while(!node_queue.empty()) {
        const BVHNode* current_node = node_queue.front();
        node_queue.pop();

        float t_max = closest_hit.didHit() ? closest_hit.distance : FAR_DISTANCE;
        if(current_node->aabb.hit(ray, EPSILON, t_max)) {
            if(!current_node->isLeaf()) {
                node_queue.push(current_node->left.get());
                node_queue.push(current_node->right.get());
            } else if(current_node->num_objects > 0) {
                for(int i = 0; i < current_node->num_objects; i++)
                    hitFunc(current_node->start_index + i, closest_hit);
            }
        }
    }
    return closest_hit;
}

template class BVHNode<RawTriangle>;
template class BVHNode<std::unique_ptr<Hittable>>;
