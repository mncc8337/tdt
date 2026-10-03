#pragma once

#include "Texture.h"

class ColorTexture: public Texture {
private:
    Color color;

public:
    ColorTexture(Color color);

    Color get(const HitInfo& rec) const;
};
