#pragma once

#include "math/Vec3.h"

Color& gammaCorrect(Color& color, float gamma);

// TODO: turn these into classes
Color& exposureTonemap(Color& color, float exposure);
Color& reihardTonemap(Color& color);
