#pragma once

#include <vector>
#include <memory>

#include "Hittable.h"
#include "Material.h"
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

    std::unique_ptr<BVHNode> bvh_root = nullptr;

public:
    template <typename T, typename... Args>
    Hittable& addObject(Args&&... args);
    // void remove_object(Hittable* object);

    template <typename T, typename... Args>
    Material& addMaterial(Args&&... args);

    template <typename T, typename... Args>
    Texture& addTexture(Args&&... args);

    template <typename T, typename... Args>
    Medium& addMedium(Args&&... args);

    const Medium& getGlobalMedium() const;
    void setGlobalMedium(Medium& medium);

    const std::vector<std::unique_ptr<Hittable>>& getObjects() const;
    const std::vector<std::unique_ptr<Material>>& getMaterials() const;
    const std::vector<std::unique_ptr<Texture>>& getTextures() const;
    const std::vector<std::unique_ptr<Medium>>& getMediums() const;

    HitInfo getClosest(const Ray& ray, Hittable*& obj) const;

    void buildBVH();

    // void save_scene(std::string path);
    // void load_scene(std::string path);
};

#include "Scene.inl"
