#pragma once

#include "HitInfo.h"
#include "misc/NamedElement.h"

class Texture: public NamedElement {
public:
    Texture(const std::string name);

    virtual Color get(const HitInfo& rec) const = 0;
};
