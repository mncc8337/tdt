#pragma once

#include <vector>
#include <memory>

#include <Hittable.h>

class Scene {
private:
    std::vector<std::unique_ptr<Hittable>> objects;
    // std::vector<std::unique_ptr<Material>> materials;
    // std::vector<std::unique_ptr<Texture>> textures;

    // std::unique_ptr<BVHNode>> bvh_root;

public:
    void add_object(std::unique_ptr<Hittable> object);
    // void remove_object(Hittable* object);

    HitInfo get_closest(const Ray& ray) const;

    // void save_scene(std::string path);
    // void load_scene(std::string path);
    // void build_bvh();
};
