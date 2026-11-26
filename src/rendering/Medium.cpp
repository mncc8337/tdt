#include "Medium.h"

Medium::Medium(unsigned id, const std::string name, const float ior):
    NamedElement(id, name),
    ior(ior) {}

const float& Medium::getIOR() const {
    return ior;
}

void Medium::setIOR(const float t) {
    ior = t;
}

float Medium::transmit(Ray& ray, Color& throughput, const float& distance) const {
    // alway success
    return distance;
}
