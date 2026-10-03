#pragma once

#include "Material.h"

class Metal: public Material {
private:
    float roughness;

public:
    Metal(Texture* texture, float roughness);

    bool scatter(Ray& ray, Color& attenuation, const HitInfo& rec) const;
};
