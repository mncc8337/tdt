#include "Mesh.h"
#include "misc/floatcmp.h"
#include "misc/meshloader.h"

Mesh::Mesh(Material* material, const std::vector<RawTriangle>& src_tris):
    Hittable(material), tris(src_tris) {
    if(tris.empty()) return;
    init_bvh();
}

Mesh::Mesh(Material* material, std::string filename):
    Hittable(material) {
    loadMeshFrom(filename, tris);
    if(tris.empty()) return;
    init_bvh();
}

void Mesh::init_bvh() {
    BVHNode* temp_root = new BVHNode(tris, 0, tris.size());
    aabb = temp_root->aabb;

    flat_bvh.resize(tris.size() * 2);
    int offset = 0;
    flatten_bvh_tree(temp_root, offset);
    flat_bvh.resize(offset);

    delete temp_root;
}

int Mesh::flatten_bvh_tree(BVHNode* node, int& offset) {
    LinearBVHNode* linear_node = &flat_bvh[offset];
    linear_node->aabb = node->aabb;

    int my_offset = offset++;

    if(node->leaf_node) {
        linear_node->primitives_offset = node->start_index;
        linear_node->num_objects = node->num_objects;
    } else {
        linear_node->num_objects = 0;
        linear_node->axis = node->axis;
        flatten_bvh_tree(node->left, offset); 
        linear_node->right_offset = flatten_bvh_tree(node->right, offset);
    }
    return my_offset;
}

HitInfo Mesh::hit(const Ray& ray) const {
    HitInfo closest_hit;

    Ray local_ray = ray;
    local_ray.origin = transform.pointApplyInverse(local_ray.origin);
    local_ray.direction = transform.dirApplyInverse(local_ray.direction);

    if(flat_bvh.empty()) return closest_hit;

    int nodes_to_visit[64];
    int to_visit_offset = 0;
    int current_node_index = 0;

    while(true) {
        const LinearBVHNode& node = flat_bvh[current_node_index];

        float aabb_t_max = DID_HIT(closest_hit) ? closest_hit.distance : FAR_DISTANCE;
        if(node.aabb.hit(local_ray, EPSILON, aabb_t_max)) {
            if(node.num_objects > 0) {
                for(int i = 0; i < node.num_objects; ++i) {
                    const auto& tri = tris[node.primitives_offset + i];

                    Vec3 tuv;
                    float tri_t_max = DID_HIT(closest_hit) ? closest_hit.distance : FAR_DISTANCE;
                    if(tri.hit(local_ray, EPSILON, tri_t_max, tuv)) {
                        closest_hit.distance = tuv.x;
                        closest_hit.hit_point = local_ray.point(tuv.x);

                        Vec3 edge1 = tri.v1 - tri.v0;
                        Vec3 edge2 = tri.v2 - tri.v0;
                        Vec3 outward_normal = edge1.cross(edge2).normalized();

                        closest_hit.front_face = local_ray.direction.dot(outward_normal) < 0.0f;
                        closest_hit.normal = closest_hit.front_face ? outward_normal : -outward_normal;
                        float u = tuv.y;
                        float v = tuv.z;
                        float w = 1.0f - u - v;
                        closest_hit.uv.x = w * tri.uv0.x + u * tri.uv1.x + v * tri.uv2.x;
                        closest_hit.uv.y = w * tri.uv0.y + u * tri.uv1.y + v * tri.uv2.y;
                    }
                }

                if(to_visit_offset == 0) break;
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

    if(DID_HIT(closest_hit)) {
        closest_hit.material = material;
        // convert to world space coordinates
        closest_hit.hit_point = transform.pointApply(closest_hit.hit_point);
        // project the hp-origin vector to the ray direction
        // since ray direction is normalized, it simply equals to its world space distance
        closest_hit.distance = (closest_hit.hit_point - ray.origin).dot(ray.direction);
        closest_hit.normal = transform.normalApply(closest_hit.normal);
    }
    return closest_hit;
}

AABB Mesh::getLocalAABB() const {
    return aabb;
}
