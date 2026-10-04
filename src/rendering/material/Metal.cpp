#include "Metal.h"
#include "misc/RNG.h"

Metal::Metal(Texture* texture, float roughness):
    Material(texture),
    roughness(roughness) {}

bool Metal::scatter(Ray& ray, Color& attenuation, const HitInfo& rec) const {
    Vec3 specular = ray.direction.reflection(rec.normal);
    Vec3 diffuse = (rec.normal + RNG::direction_unnormalized()).normalized();

    attenuation = texture->get(rec);

    ray.direction = specular.lerp(diffuse, roughness);
    // move the origin a little bit forward so it does not lies on the sphere surface
    ray.origin = rec.hit_point + rec.normal * 0.001f;

    return true;
}
