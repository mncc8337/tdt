#pragma once

#include "Material.h"

class Light: public Material {
private:
    float emission_strength;
    Color color;

public:
    Light(const std::string name, Texture* texture, const float emission_strength, const Color color);

    float& getEmissionStrength();
    const float& getEmissionStrength() const;

    Color& getColor();
    const Color& getColor() const;

    Color emitted(const HitInfo& rec) const override;

    bool isEmissive() const override;

    ScatterResult scatter(
        Ray& ray,
        Color& attenuation,
        const HitInfo& rec,
        const Medium& origin_medium
    ) const override;
};


