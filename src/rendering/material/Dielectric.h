#pragma once

#include "Material.h"

class Dielectric: public Material {
private:
    float ior;

public:
    Dielectric(Texture* texture, float ior);

    bool scatter(Ray& ray, Color& attenuation, const HitInfo& rec) const override;
};

