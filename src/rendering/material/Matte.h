#pragma once

#include "Material.h"

class Matte: public Material {
public:
    Matte(unsigned id, const std::string name, Texture* texture);

    ScatterResult scatter(
        Ray& ray,
        Color& attenuation,
        const HitInfo& rec,
        const Medium& origin_medium
    ) const override;

    Color emitted(const HitInfo& rec) const override;

    bool hasNEE() const override;

    Color BRDF(const Vec3& in_dir, const Vec3& out_dir, const HitInfo& rec) const override;
};

