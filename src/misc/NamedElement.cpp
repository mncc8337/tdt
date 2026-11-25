#include "NamedElement.h"
#include <print>

unsigned NamedElement::id_count = 0;

NamedElement::NamedElement(std::string name):
    name(name),
    id(id_count++) {
    std::println("created element {} with name \"{}\"", id, name.c_str());
}

NamedElement::~NamedElement() {
    std::println("deleted element {} with name \"{}\"", id, name.c_str());
}

const std::string NamedElement::getName() const {
    return name;
}

const unsigned NamedElement::getID() const {
    return id;
}
