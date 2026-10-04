#pragma once

#include "HitInfo.h"

class Texture {
public:
    virtual Color get(const HitInfo& rec) const = 0;
};
