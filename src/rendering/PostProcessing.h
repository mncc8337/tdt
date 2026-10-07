#pragma once

#include "math/Vec3.h"

Color& clamp(Color& color);
Color& gamma_correct(Color& color, float gamma);

// TODO: turn these into classes
Color& exposure_tonemap(Color& color, float exposure);
Color& reihard_tonemap(Color& color);
