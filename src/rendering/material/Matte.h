#pragma once

#include "Material.h"

class Matte: public Material {
public:
    Matte(Texture* texture);

    bool scatter(Ray& ray, Color& attenuation, const HitInfo& rec) const;
};

