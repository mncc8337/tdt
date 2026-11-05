#include "BVHNode.h"
#include "misc/floatcmp.h"
#include <memory>
#include <vector>

bool BVHNode::isLeaf() const {
    return !left and !right;
}

HitInfo BVHNode::traverse(
    const Ray& ray,
    std::function<void(const int, HitInfo&)> hitFunc
) const {
    HitInfo closest_hit;

    // impl using std::vector to utilize the reserve() method
    std::vector<const BVHNode*> node_stack;
    node_stack.reserve(64);

    const BVHNode* current_node = this;

    while(true) {
        float t_max = closest_hit.didHit() ? closest_hit.distance : FAR_DISTANCE;
        if(current_node->aabb.hit(ray, EPSILON, t_max)) {
            if(!current_node->isLeaf()) {
                node_stack.push_back(current_node->right.get());
                current_node = current_node->left.get();
                continue;
            } else if(current_node->num_objects > 0) {
                for(int i = 0; i < current_node->num_objects; i++)
                    hitFunc(current_node->start_index + i, closest_hit);
            }
        }

        if(node_stack.empty()) break;
        current_node = node_stack.back();
        node_stack.pop_back();
    }

    return closest_hit;
}
