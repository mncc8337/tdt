#pragma once

#include "Material.h"

class Metal: public Material {
private:
    float roughness;

public:
    Metal(unsigned id, const std::string name, Texture* texture, float roughness);

    float& getRoughness();
    const float& getRoughness() const;

    ScatterResult scatter(
        Ray& ray,
        Color& attenuation,
        const HitInfo& rec,
        const Medium& origin_medium
    ) const override;

    Color emitted(const HitInfo& rec) const override;
};
