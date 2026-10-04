#include "Matte.h"
#include "misc/floatcmp.h"
#include "misc/RNG.h"

Matte::Matte(Texture* texture):
    Material(texture) {}

bool Matte::scatter(Ray& ray, Color& attenuation, const HitInfo& rec) const {
    Vec3 scatter_direction = rec.normal + RNG::direction_normalized();

    if(fequal0(scatter_direction.length_squared())) {
        scatter_direction = rec.normal;
    }

    ray.direction = scatter_direction.normalized();
    ray.origin = rec.hit_point + rec.normal * RAY_ORIGIN_OFFSET;

    attenuation = texture->get(rec); 

    return true;
}
