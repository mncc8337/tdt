#pragma once

#include "math/Ray.h"
#include "Texture.h"
#include "HitInfo.h"

class Material {
protected:
    Texture* texture;

public:
    Material(Texture* texture);

    virtual bool scatter(
        Ray& ray,
        Color& attenuation,
        const HitInfo& rec
    ) const = 0;

    virtual Color emitted(
        const float uvx,
        const float uvy,
        const Vec3 hit_point
    ) const;
};
