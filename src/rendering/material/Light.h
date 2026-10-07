#pragma once

#include "Material.h"

class Light: public Material {
private:
    float emission_strength;
    Color color;

public:
    Light(Texture* texture, const float emission_strength, const Color color);

    Color emitted(
        const Vec2 uv,
        const Vec3 hit_point
    ) const override;
};


