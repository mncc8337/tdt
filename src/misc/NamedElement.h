#pragma once

#include <string>

class NamedElement {
private:
    std::string name;

public:
    NamedElement(std::string name);

    const std::string getName() const;
};
