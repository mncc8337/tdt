#pragma once

#include "Texture.h"

class ColorTexture: public Texture {
private:
    Color color;

public:
    ColorTexture(unsigned id, const std::string name, Color color);

    Color get(const HitInfo& rec) const override;

    const Color& getColor() const;
    void setColor(const Color c);
};
