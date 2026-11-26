#pragma once

#include "math/Vec3.h"
#include "math/Ray.h"
#include "misc/NamedElement.h"

class Medium: public NamedElement {
private:
    float ior;

public:
    Medium(unsigned id, const std::string name, const float ior = 1.0f);
    virtual ~Medium() = default;

    const float& getIOR() const;
    void setIOR(const float t);

    // Try to advance the ray by up to `distance`.
    // On success, returns `distance` and multiplies `throughput` by the
    // transmittance of the segment. If the medium scatters before the ray
    // reaches that distance, moves the ray to the scatter point, picks a new
    // direction, multiplies `throughput` by the scattering weight, and returns
    // the travelled distance (which is smaller than `distance`).
    // NOTE:
    // we dont move the ray when succeed because we most likely to move the ray afterward
    // via scatter() in the pipeline
    virtual float transmit(Ray& ray, Color& throughput, const float& distance) const;
};
