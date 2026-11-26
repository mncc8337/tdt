#pragma once
#include "Scene.h"
#include <utility>
#include <algorithm>

inline unsigned Scene::nextID() {
    return next_id++;
}

inline void Scene::reserveID(unsigned id) {
    next_id = std::max(next_id, id + 1u);
}

template <typename T, typename... Args>
const Hittable& Scene::addObject(Args&&... args) {
    return addObject<T>(nextID(), std::forward<Args>(args)...);
}

template <typename T, typename... Args>
const Hittable& Scene::addObject(unsigned id, Args&&... args) {
    auto ptr = std::make_unique<T>(id, std::forward<Args>(args)...);
    auto& ref = *ptr;
    reserveID(id);
    objects.push_back(std::move(ptr));

    if(ref.getMaterial() and ref.getMaterial()->isEmissive()) {
        lights.push_back(&ref);
    }

    return ref;
}

template <typename T, typename... Args>
Material& Scene::addMaterial(Args&&... args) {
    return addMaterial<T>(nextID(), std::forward<Args>(args)...);
}

template <typename T, typename... Args>
Material& Scene::addMaterial(unsigned id, Args&&... args) {
    auto ptr = std::make_unique<T>(id, std::forward<Args>(args)...);
    auto& ref = *ptr;
    reserveID(id);
    materials.push_back(std::move(ptr));
    return ref;
}

template <typename T, typename... Args>
Texture& Scene::addTexture(Args&&... args) {
    return addTexture<T>(nextID(), std::forward<Args>(args)...);
}

template <typename T, typename... Args>
Texture& Scene::addTexture(unsigned id, Args&&... args) {
    auto ptr = std::make_unique<T>(id, std::forward<Args>(args)...);
    auto& ref = *ptr;
    reserveID(id);
    textures.push_back(std::move(ptr));
    return ref;
}

template <typename T, typename... Args>
Medium& Scene::addMedium(Args&&... args) {
    return addMedium<T>(nextID(), std::forward<Args>(args)...);
}

template <typename T, typename... Args>
Medium& Scene::addMedium(unsigned id, Args&&... args) {
    auto ptr = std::make_unique<T>(id, std::forward<Args>(args)...);
    auto& ref = *ptr;
    reserveID(id);
    mediums.push_back(std::move(ptr));
    return ref;
}

template <typename T>
T* Scene::findOwned(const std::vector<std::unique_ptr<T>>& items, const T* item) {
    if(!item) return nullptr;

    const auto found = std::find_if(
        items.begin(),
        items.end(),
        [item](const std::unique_ptr<T>& owned) { return owned.get() == item; }
    );
    return found == items.end() ? nullptr : found->get();
}

template <typename T, typename Predicate>
void Scene::eraseOwnedIf(std::vector<std::unique_ptr<T>>& items, Predicate&& predicate) {
    items.erase(
        std::remove_if(
            items.begin(),
            items.end(),
            [&predicate](const std::unique_ptr<T>& owned) {
                return predicate(owned.get());
            }
        ),
        items.end()
    );
}
