#pragma once

#include <vector>
#include <memory>

#include "Hittable.h"
#include "accel/BVHNode.h"

class Scene {
private:
    std::vector<std::unique_ptr<Hittable>> objects;
    std::vector<std::unique_ptr<Material>> materials;
    std::vector<std::unique_ptr<Texture>> textures;

    std::vector<LinearBVHNode> flat_bvh;

    int flatten_bvh_tree(BVHNode* node, int& offset);

public:
    Hittable& add_object(std::unique_ptr<Hittable> object);
    // void remove_object(Hittable* object);

    Material& add_material(std::unique_ptr<Material> material);

    Texture& add_texture(std::unique_ptr<Texture> texture);

    HitInfo get_closest(const Ray& ray) const;

    // void save_scene(std::string path);
    // void load_scene(std::string path);

    void build_bvh();
};
