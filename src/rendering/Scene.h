#pragma once

#include <vector>
#include <memory>

#include "Texture.h"
#include "Medium.h"
#include "dsa/BVHNode.h"

class Scene {
private:
    std::vector<std::unique_ptr<Hittable>> objects;
    std::vector<std::unique_ptr<Material>> materials;
    std::vector<std::unique_ptr<Texture>> textures;
    std::vector<std::unique_ptr<Medium>> mediums;

    Medium* global_medium = nullptr;

    std::unique_ptr<BVHNode<std::unique_ptr<Hittable>>> bvh_root = nullptr;

public:
    Hittable& addObject(std::unique_ptr<Hittable> object);
    // void remove_object(Hittable* object);

    Material& addMaterial(std::unique_ptr<Material> material);
    Texture& addTexture(std::unique_ptr<Texture> texture);
    Medium& addMedium(std::unique_ptr<Medium> medium);

    Medium*& getGlobalMedium();
    const Medium*const& getGlobalMedium() const;

    const std::vector<std::unique_ptr<Hittable>>& getObjects() const;
    const std::vector<std::unique_ptr<Material>>& getMaterials() const;
    const std::vector<std::unique_ptr<Texture>>& getTextures() const;
    const std::vector<std::unique_ptr<Medium>>& getMediums() const;

    HitInfo getClosest(const Ray& ray, Hittable*& obj) const;

    void buildBVH();

    // void save_scene(std::string path);
    // void load_scene(std::string path);
};
