#include "Medium.h"

Medium::Medium(const std::string name, const float ior): name(name), ior(ior) {}

float& Medium::getIOR() {
    return ior;
}

const float& Medium::getIOR() const {
    return ior;
}

Color Medium::transmit(const float distance) const {
    return Color(1);
}

std::string& Medium::getName() {
    return name;
}

const std::string& Medium::getName() const {
    return name;
}
