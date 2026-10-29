#pragma once

#include "math/Vec3.h"
#include "misc/NamedElement.h"

class Medium: public NamedElement {
private:
    float ior;

public:
    Medium(const std::string name, const float ior = 1.0f);

    const float& getIOR() const;
    void setIOR(const float t);

    virtual Color transmit(const float distance) const;
};
