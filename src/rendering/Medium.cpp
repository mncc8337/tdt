#include "Medium.h"

Medium::Medium(const float ior): ior(ior) {}

float& Medium::getIOR() {
    return ior;
}

const float& Medium::getIOR() const {
    return ior;
}

Color Medium::transmit(const float distance) const {
    return Color(1);
}
