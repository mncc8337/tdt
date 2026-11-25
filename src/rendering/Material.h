#pragma once

#include "math/Ray.h"
#include "misc/NamedElement.h"
#include "Texture.h"
#include "Medium.h"
#include "HitInfo.h"

// FIXME:
// the name material is misleading
// since we only handle surface scattering/emission here
// consider renaming it to Surface or Texture (and rename Texture to something else)
// or move the scatter method to Texture and only treat Material as the container for
// Medium and Texture

enum class ScatterResult {
    Terminated, // path ends here (hit an emitter, absorbed, or no valid continuation)
    Scattered,  // ray continues within the same medium
    Refracted   // ray crossed the surface boundary (enter if front_face, else exit)
};

class Material: public NamedElement {
protected:
    Texture* texture;
    Medium* medium;

public:
    Material(const std::string name, Texture* texture, Medium* medium);
    virtual ~Material() = default;

    virtual ScatterResult scatter(
        Ray& ray,
        Color& attenuation,
        const HitInfo& rec,
        const Medium& origin_medium
    ) const = 0;

    virtual Color emitted(const HitInfo& rec) const = 0;

    virtual bool isEmissive() const;

    virtual bool hasNEE() const;
    virtual Color BRDF(const Vec3& in_dir, const Vec3& out_dir, const HitInfo& rec) const;

    Texture* getTexture() const;
    void setTexture(Texture* texture);

    Medium* getMedium() const;
    void setMedium(Medium* medium);
};
