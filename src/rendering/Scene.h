#pragma once

#include <vector>
#include <memory>
#include <filesystem>

#include "Hittable.h"
#include "Material.h"
#include "Texture.h"
#include "Medium.h"
#include "LightSample.h"
#include "dsa/BVHNode.h"

#define SCENE_BVH_THRESHOLD 2

class SceneLoader;

class Scene {
    // the serializer needs direct access to the owned vectors and the id counter
    friend class SceneLoader;

private:
    std::vector<std::unique_ptr<Hittable>> objects;
    std::vector<std::unique_ptr<Material>> materials;
    std::vector<std::unique_ptr<Texture>> textures;
    std::vector<std::unique_ptr<Medium>> mediums;

    std::vector<Hittable*> lights;

    Medium* global_medium = nullptr;

    std::unique_ptr<BVHNode> bvh_root = nullptr;

    // ids are per-scene now, no longer a global counter in NamedElement
    unsigned next_id = 0;

    // assets unpacked from a saved scene live in the system temp directory;
    // the paths are tracked here so they can be cleaned up
    std::vector<std::filesystem::path> temp_assets;

    template <typename Item>
    Item* findOwned(const std::vector<std::unique_ptr<Item>>& items, const Item* item);

    template <typename Item, typename Predicate>
    void eraseOwnedIf(std::vector<std::unique_ptr<Item>>& items, Predicate&& predicate);

    // keep the counter ahead of ids restored from a scene file
    void reserveID(unsigned id);

public:
    ~Scene();

    // reserve and return the next id (for code that wants to pass an id explicitly)
    unsigned nextID();

    template <typename T, typename... Args>
    const Hittable& addObject(Args&&... args);
    // explicit id, used by the loader (and by anyone restoring saved elements)
    template <typename T, typename... Args>
    const Hittable& addObject(unsigned id, Args&&... args);
    void remove_object(const Hittable* object);
    void set_transform(const Hittable* object, const Transform& transform);
    void set_material(const Hittable* object, const Material* material);

    template <typename T, typename... Args>
    Material& addMaterial(Args&&... args);
    template <typename T, typename... Args>
    Material& addMaterial(unsigned id, Args&&... args);
    void remove_material(const Material* material);
    void set_texture(const Material* material, const Texture* texture);
    void set_medium(const Material* material, const Medium* medium);

    template <typename T, typename... Args>
    Texture& addTexture(Args&&... args);
    template <typename T, typename... Args>
    Texture& addTexture(unsigned id, Args&&... args);
    void remove_texture(const Texture* texture);

    template <typename T, typename... Args>
    Medium& addMedium(Args&&... args);
    template <typename T, typename... Args>
    Medium& addMedium(unsigned id, Args&&... args);
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

    // Remove every owned element and unpacked asset. Called by SceneLoader
    // after the file has been fully parsed, so a failed load leaves the
    // current scene untouched.
    void clear();
};

#include "Scene.inl"
