#include "Scene.h"
#include "misc/floatcmp.h"

Hittable& Scene::addObject(std::unique_ptr<Hittable> object) {
    objects.push_back(std::move(object));
    Hittable* obj = objects.back().get();
    return *obj;
}

Material& Scene::addMaterial(std::unique_ptr<Material> material) {
    materials.push_back(std::move(material));
    return *materials.back().get();
}

Texture& Scene::addTexture(std::unique_ptr<Texture> texture) {
    textures.push_back(std::move(texture));
    return *textures.back().get();
}

const std::vector<std::unique_ptr<Hittable>>& Scene::getObjects() const {
    return objects;
}

HitInfo Scene::getClosest(const Ray& ray, Hittable*& obj) const {
    HitInfo closest_hit;
    obj = nullptr;
    if(flat_bvh.empty()) return closest_hit;

    int nodes_to_visit[64];
    int to_visit_offset = 0;
    
    int current_node_index = 0;

    while(true) {
        const LinearBVHNode& node = flat_bvh[current_node_index];

        if(node.aabb.hit(ray, EPSILON, DID_HIT(closest_hit) ? closest_hit.distance : FAR_DISTANCE)) {
            if(node.num_objects > 0) {
                for (int i = 0; i < node.num_objects; ++i) {
                    HitInfo temp_hit = objects[node.primitives_offset + i]->hit(ray);

                    if (DID_HIT(temp_hit)) {
                        if(!DID_HIT(closest_hit) or temp_hit.distance < closest_hit.distance) {
                            closest_hit = temp_hit;
                            obj = objects[node.primitives_offset + i].get();
                        }
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

    return closest_hit;
}

void Scene::buildBVH() {
    BVHNode* temp_root = new BVHNode(objects, 0, objects.size());
    flat_bvh.resize(objects.size() * 2);

    int offset = 0;
    flattenBVH(temp_root, offset);
    
    flat_bvh.resize(offset);
    delete temp_root;
}

int Scene::flattenBVH(BVHNode* node, int& offset) {
    LinearBVHNode* linear_node = &flat_bvh[offset];
    linear_node->aabb = node->aabb;
    
    int my_offset = offset++;

    if(node->leaf_node) {
        linear_node->primitives_offset = node->start_index;
        linear_node->num_objects = node->num_objects;
    } else {
        linear_node->num_objects = 0;
        linear_node->axis = node->axis;
        flattenBVH(node->left, offset); 
        linear_node->right_offset = flattenBVH(node->right, offset);
    }
    return my_offset;
}
