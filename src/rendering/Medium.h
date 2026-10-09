#pragma once

#include "math/Vec3.h"

class Medium {
private:
    float ior;

public:
    Medium(const float ior = 1.0f);

    float& getIOR();
    const float& getIOR() const;
    virtual Color transmit(const float distance) const;
};
