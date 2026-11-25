#pragma once
#include "Scene.h"
#include <utility>
#include <algorithm>

template <typename T, typename... Args>
const Hittable& Scene::addObject(Args&&... args) {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto& ref = *ptr;
    objects.push_back(std::move(ptr));

    if(ref.getMaterial()->isEmissive()) {
        lights.push_back(&ref);
    }

    return ref;
}

template <typename T, typename... Args>
Material& Scene::addMaterial(Args&&... args) {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto& ref = *ptr;
    materials.push_back(std::move(ptr));
    return ref;
}

template <typename T, typename... Args>
Texture& Scene::addTexture(Args&&... args) {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto& ref = *ptr;
    textures.push_back(std::move(ptr));
    return ref;
}

template <typename T, typename... Args>
Medium& Scene::addMedium(Args&&... args) {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto& ref = *ptr;
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

