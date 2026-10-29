#include "Medium.h"

Medium::Medium(const std::string name, const float ior):
    NamedElement(name),
    ior(ior) {}

const float& Medium::getIOR() const {
    return ior;
}

void Medium::setIOR(const float t) {
    ior = t;
}

Color Medium::transmit(const float distance) const {
    return Color(1);
}
