#pragma once

#include "Texture.h"
// TODO: make a universal image loader class
#include <SFML/Graphics/Image.hpp>

class ImageTexture: public Texture {
private:
    sf::Image image;

public:
    ImageTexture(const std::string name, const std::filesystem::path &filename);

    Color get(const HitInfo& rec) const override;
};

