#pragma once

#include <vector>
#include <memory>

#include <Hittable.h>

class Scene {
private:
    std::vector<std::unique_ptr<Hittable>> objects;
    std::vector<std::unique_ptr<Material>> materials;
    std::vector<std::unique_ptr<Texture>> textures;

    // std::unique_ptr<BVHNode>> bvh_root;

public:
    Hittable& add_object(std::unique_ptr<Hittable> object);
    // void remove_object(Hittable* object);

    Material& add_material(std::unique_ptr<Material> material);

    Texture& add_texture(std::unique_ptr<Texture> texture);

    HitInfo get_closest(const Ray& ray) const;

    // void save_scene(std::string path);
    // void load_scene(std::string path);
    // void build_bvh();
};
