#pragma once

#include "HitInfo.h"
#include "misc/NamedElement.h"

class Texture: public NamedElement {
public:
    Texture(unsigned id, const std::string name);
    virtual ~Texture() = default;

    virtual Color get(const HitInfo& rec) const = 0;
};
