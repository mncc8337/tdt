#pragma once

#include "Material.h"
#include "rendering/medium/Homogeneous.h"

class FineParticle: public Material {
private:

public:
    FineParticle(unsigned id, const std::string name, Texture* texture, Homogeneous* medium);

    ScatterResult scatter(
        Ray& ray,
        Color& attenuation,
        const HitInfo& rec,
        const Medium& origin_medium
    ) const override;

    Color emitted(const HitInfo& rec) const override;
};
