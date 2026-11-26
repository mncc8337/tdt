#include "NamedElement.h"
#include <print>

NamedElement::NamedElement(unsigned id, std::string name):
    name(name),
    id(id) {
    std::println("created element {} with name \"{}\"", id, name.c_str());
}

NamedElement::~NamedElement() {
    std::println("deleted element {} with name \"{}\"", id, name.c_str());
}

const std::string NamedElement::getName() const {
    return name;
}

void NamedElement::setName(const std::string& name) {
    this->name = name;
}

const unsigned NamedElement::getID() const {
    return id;
}
