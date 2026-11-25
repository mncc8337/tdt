#pragma once

#include <vector>
#include <memory>

#include "Hittable.h"
#include "Material.h"
#include "Texture.h"
#include "Medium.h"
#include "LightSample.h"
#include "dsa/BVHNode.h"

#define SCENE_BVH_THRESHOLD 2

class Scene {
private:
    std::vector<std::unique_ptr<Hittable>> objects;
    std::vector<std::unique_ptr<Material>> materials;
    std::vector<std::unique_ptr<Texture>> textures;
    std::vector<std::unique_ptr<Medium>> mediums;

    std::vector<Hittable*> lights;

    Medium* global_medium = nullptr;

    std::unique_ptr<BVHNode> bvh_root = nullptr;

    template <typename Item>
    Item* findOwned(const std::vector<std::unique_ptr<Item>>& items, const Item* item);

    template <typename Item, typename Predicate>
    void eraseOwnedIf(std::vector<std::unique_ptr<Item>>& items, Predicate&& predicate);

public:
    template <typename T, typename... Args>
    const Hittable& addObject(Args&&... args);
    void remove_object(const Hittable* object);
    void set_transform(const Hittable* object, const Transform& transform);
    void set_material(const Hittable* object, const Material* material);

    template <typename T, typename... Args>
    Material& addMaterial(Args&&... args);
    void remove_material(const Material* material);
    void set_texture(const Material* material, const Texture* texture);
    void set_medium(const Material* material, const Medium* medium);

    template <typename T, typename... Args>
    Texture& addTexture(Args&&... args);
    void remove_texture(const Texture* texture);

    template <typename T, typename... Args>
    Medium& addMedium(Args&&... args);
    void remove_medium(const Medium* medium);

    const Medium& getGlobalMedium() const;
    void setGlobalMedium(Medium& medium);

    const std::vector<std::unique_ptr<Hittable>>& getObjects() const;
    const std::vector<std::unique_ptr<Material>>& getMaterials() const;
    const std::vector<std::unique_ptr<Texture>>& getTextures() const;
    const std::vector<std::unique_ptr<Medium>>& getMediums() const;

    HitInfo getClosest(const Ray& ray, Hittable*& obj) const;

    // Sample one light uniformly and return its contribution data.
    bool sampleLight(const Vec3& from, LightSample& sample) const;

    void buildBVH();

    // void save_scene(std::string path);
    // void load_scene(std::string path);
};

#include "Scene.inl"
