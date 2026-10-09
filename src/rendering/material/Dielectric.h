#pragma once

#include "Material.h"

class Dielectric: public Material {
private:

public:
    Dielectric(Texture* texture, Medium* medium);

    ScatterResult scatter(
        Ray& ray,
        Color& attenuation,
        const HitInfo& rec,
        const Medium& origin_medium
    ) const override;
};

