#include "BVHNode.h"
#include "misc/floatcmp.h"
#include <memory>
#include <queue>

bool BVHNode::isLeaf() const {
    return !left and !right;
}

HitInfo BVHNode::traverse(
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
