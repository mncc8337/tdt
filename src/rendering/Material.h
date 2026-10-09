#pragma once

#include "math/Ray.h"
#include "Texture.h"
#include "Medium.h"
#include "HitInfo.h"

enum class ScatterResult {
    Terminated, // path ends here (hit an emitter, absorbed, or no valid continuation)
    Scattered,  // ray continues within the same medium
    Refracted   // ray crossed the surface boundary (enter if front_face, else exit)
};

class Material {
protected:
    Texture* texture;
    Medium* medium;

public:
    Material(Texture* texture, Medium* medium);

    virtual ScatterResult scatter(
        Ray& ray,
        Color& attenuation,
        const HitInfo& rec,
        const Medium& origin_medium
    ) const;

    virtual Color emitted(const HitInfo& rec) const;

    Texture* getTexture() const;
    Medium* getMedium() const;
};
