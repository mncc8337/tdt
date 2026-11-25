#pragma once

#include <string>

class NamedElement {
private:
    std::string name;
    static unsigned id_count;
    const unsigned id;

public:
    NamedElement(std::string name);
    ~NamedElement();

    const std::string getName() const;
    const unsigned getID() const;
};
