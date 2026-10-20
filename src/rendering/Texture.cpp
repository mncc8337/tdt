#include "Texture.h"

Texture::Texture(const std::string name): name(name) {}

std::string& Texture::getName() {
    return name;
}

const std::string& Texture::getName() const {
    return name;
}
