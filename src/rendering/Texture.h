#pragma once

#include "HitInfo.h"
#include <string>

class Texture {
protected:
    std::string name;

public:
    Texture(const std::string name);

    virtual Color get(const HitInfo& rec) const = 0;

    std::string& getName();
    const std::string& getName() const;
};
