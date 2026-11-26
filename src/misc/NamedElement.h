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
    void setName(const std::string& name);
    const unsigned getID() const;
};
