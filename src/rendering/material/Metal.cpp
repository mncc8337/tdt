#include "Metal.h"
#include "misc/floatcmp.h"
#include "misc/RNG.h"

Metal::Metal(Texture* texture, float roughness):
    Material(texture),
    roughness(roughness) {}

bool Metal::scatter(Ray& ray, Color& attenuation, const HitInfo& rec) const {
    Vec3 specular = ray.direction.reflection(rec.normal);

    if(fequal0(roughness))
        ray.direction = specular;
    else
        ray.direction = (specular + RNG::directionUnnormalized() * roughness).normalized();

    if(ray.direction.dot(rec.normal) <= 0.0f) {
        return false; 
    }

    attenuation = texture->get(rec);
    ray.origin = rec.hit_point + rec.normal * RAY_ORIGIN_OFFSET;

    return true;
}

float& Metal::getRoughness() {
    return roughness;
}

const float& Metal::getRoughness() const {
    return roughness;
}
