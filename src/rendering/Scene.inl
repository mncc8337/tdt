#pragma once
#include "Scene.h"

template <typename T, typename... Args>
Hittable& Scene::addObject(Args&&... args) {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto& ref = *ptr;
    objects.push_back(std::move(ptr));
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
