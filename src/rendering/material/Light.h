#pragma once

#include "Material.h"

class Light: public Material {
private:
    float emission_strength;
    Color color;

public:
    Light(Texture* texture, const float emission_strength, const Color color);

    float& getEmissionStrength();
    const float& getEmissionStrength() const;

    Color& getColor();
    const Color& getColor() const;

    Color emitted(const HitInfo& rec) const override;
};


