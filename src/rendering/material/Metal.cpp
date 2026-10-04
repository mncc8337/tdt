#include "Metal.h"
#include "misc/floatcmp.h"
#include "misc/RNG.h"

Metal::Metal(Texture* texture, float roughness):
    Material(texture),
    roughness(roughness) {}

bool Metal::scatter(Ray& ray, Color& attenuation, const HitInfo& rec) const {
    Vec3 specular = ray.direction.reflection(rec.normal);

    ray.direction = (specular + RNG::direction_unnormalized() * roughness).normalized();

    if(ray.direction.dot(rec.normal) <= 0.0f) {
        return false; 
    }

    attenuation = texture->get(rec);
    // move the origin a little bit forward so it does not lies on the sphere surface
    ray.origin = rec.hit_point + rec.normal * RAY_ORIGIN_OFFSET;

    return true;
}
