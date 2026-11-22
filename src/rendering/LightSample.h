#pragma once

#include "math/Vec3.h"

class Hittable;

// result of sampling one light for next-event estimation.
//
// pdf is the solid-angle density of the whole sampling procedure:
//   P(select this light) * p_light(omega)
// where p_light(omega) already includes the area-to-solid-angle conversion.
// it does not include visibility or the BSDF.
struct LightSample {
    Vec3 light_direction; // direction from the shading point to the light
    float light_distance = 0.0f; // distance from the shading point to the sampled point
    float pdf = 0.0f; // solid-angle pdf, including the light-selection probability
    Color emitted_radiance; // emitted radiance at the sampled point
    const Hittable* light = nullptr; // the sampled light, so shadow rays can ignore it
};
