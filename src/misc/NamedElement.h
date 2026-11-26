#pragma once

#include <string>

class NamedElement {
private:
    std::string name;
    const unsigned id;

public:
    // ids are owned by the Scene that creates the element, so a loaded
    // element can keep the id it had when the scene was saved
    NamedElement(unsigned id, std::string name);
    virtual ~NamedElement();

    const std::string getName() const;
    void setName(const std::string& name);
    const unsigned getID() const;
};
