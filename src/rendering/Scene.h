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

    int flattenBVH(BVHNode* node, int& offset);

public:
    Hittable& addObject(std::unique_ptr<Hittable> object);
    // void remove_object(Hittable* object);

    Material& addMaterial(std::unique_ptr<Material> material);

    Texture& addTexture(std::unique_ptr<Texture> texture);

    HitInfo getClosest(const Ray& ray, Hittable*& obj) const;

    const std::vector<std::unique_ptr<Hittable>>& getObjects() const;

    // void save_scene(std::string path);
    // void load_scene(std::string path);

    void buildBVH();
};
