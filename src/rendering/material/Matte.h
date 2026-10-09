#pragma once

#include "Material.h"

class Matte: public Material {
public:
    Matte(Texture* texture);

    ScatterResult scatter(
        Ray& ray,
        Color& attenuation,
        const HitInfo& rec,
        const Medium& origin_medium
    ) const override;
};

