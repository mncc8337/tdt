#pragma once

#include "math/Vec3.h"
#include <string>

class Medium {
private:
    float ior;
    std::string name;

public:
    Medium(const std::string name, const float ior = 1.0f);

    std::string& getName();
    const std::string& getName() const;

    float& getIOR();
    const float& getIOR() const;
    virtual Color transmit(const float distance) const;
};
