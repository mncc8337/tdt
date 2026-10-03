#pragma once

#include "math/Color.h"
#include "HitInfo.h"

class Texture {
public:
    virtual Color get(const HitInfo& rec) const = 0;
};
