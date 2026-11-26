#pragma once

#include "Texture.h"
// TODO: make a universal image loader class
#include <SFML/Graphics/Image.hpp>

class ImageTexture: public Texture {
private:
    sf::Image image;
    // kept for scene serialization
    std::filesystem::path filename;

public:
    ImageTexture(unsigned id, const std::string name, const std::filesystem::path &filename);

    const std::filesystem::path& getFilename() const;

    Color get(const HitInfo& rec) const override;
};

