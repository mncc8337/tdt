#include "Dielectric.h"
#include "misc/RNG.h"
#include "misc/floatcmp.h"

static float reflectance(float cosine, float ri) {
    // use Schlick's approximation for reflectance.
    float r0 = (1-ri) / (1+ri);
    r0 *= r0;
    float icos = 1 - cosine;
    return r0 + (1 - r0) * icos * icos * icos * icos * icos;
}

Dielectric::Dielectric(Texture* texture, float ior):
    Material(texture),
    ior(ior) {}

bool Dielectric::scatter(Ray& ray, Color& attenuation, const HitInfo& rec) const {
    Vec3 dir = ray.getDirection().normalized();

    float env_ior = 1.0; // TODO: move this into scene defs
    float ior_ratio = env_ior / ior;
    if(!rec.front_face) {
        ior_ratio = ior / env_ior;
    }

    float cos_theta = std::min(-dir.dot(rec.normal), 1.0f);
    bool should_reflect = reflectance(cos_theta, ior_ratio) > RNG::uniform();
    bool cannot_refract = ior_ratio * ior_ratio * (1.0 - cos_theta * cos_theta) > 1.0;

    if((cannot_refract or should_reflect) and !fequal(ior_ratio, 1.0f)) {
        ray.setDirection(dir.reflection(rec.normal));
        ray.setOrigin(rec.hit_point + rec.normal * 0.001f);
    } else {
        ray.setDirection(dir.refraction(rec.normal, ior_ratio));
        ray.setOrigin(rec.hit_point - rec.normal * 0.001f);
    }

    attenuation = texture->get(rec);

    return true;
}
