#pragma once

#include "Texture.h"

class ColorTexture: public Texture {
private:
    Color color;

public:
    ColorTexture(const std::string name, Color color);

    Color get(const HitInfo& rec) const;

    Color& getColor();
};
