#include "accel/BVHNode.h"

int BVHNode::flatten(
    std::vector<LinearBVHNode>& flat_bvh,
    BVHNode* node,
    int& offset
) {
    LinearBVHNode* linear_node = &flat_bvh[offset];
    linear_node->aabb = node->aabb;

    int my_offset = offset++;

    if(node->leaf_node) {
        linear_node->primitives_offset = node->start_index;
        linear_node->num_objects = node->num_objects;
    } else {
        linear_node->num_objects = 0;
        linear_node->axis = node->axis;
        int l = flatten(flat_bvh, node->left, offset);
        int r = flatten(flat_bvh, node->right, offset);
        linear_node->right_offset = r;
    }
    return my_offset;
}

HitInfo BVHNode::traverse(
    const std::vector<LinearBVHNode>& flat_bvh,
    const Ray& ray,
    std::function<void(const int, HitInfo&)> hitFunc
) {
    HitInfo closest_hit;
    if (flat_bvh.empty()) return closest_hit;

    int nodes_to_visit[64];
    int to_visit_offset = 0;
    int current_node_index = 0;

    while(true) {
        const LinearBVHNode& node = flat_bvh[current_node_index];

        float t_max = DID_HIT(closest_hit) ? closest_hit.distance : FAR_DISTANCE;
        if(node.aabb.hit(ray, EPSILON, t_max)) {
            if(node.num_objects > 0) {
                for(int i = 0; i < node.num_objects; ++i)
                    hitFunc(node.primitives_offset + i, closest_hit);

                if (to_visit_offset == 0) break;
                current_node_index = nodes_to_visit[--to_visit_offset];
            } else {
                nodes_to_visit[to_visit_offset++] = node.right_offset;
                current_node_index = current_node_index + 1;
            }
        } else {
            if(to_visit_offset == 0) break;
            current_node_index = nodes_to_visit[--to_visit_offset];
        }
    }
    return closest_hit;
}
