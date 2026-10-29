#include "NamedElement.h"

NamedElement::NamedElement(std::string name):
    name(name) {}

const std::string NamedElement::getName() const {
    return name;
}
