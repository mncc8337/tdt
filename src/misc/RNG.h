#pragma once

#include <random>
#include "math/Vec3.h"

class RNG {
private:
    static std::uniform_real_distribution<float> uniform_dist;
    static std::normal_distribution<float> normal_dist;
    static std::mt19937 generator;

public:
    static float uniform();
    static float uniform(float from, float to);
    static float normal();
    static Vec3 direction_unnormalized();
    static Vec3 direction_normalized();
    static Vec3 direction_in_unit_sphere();
    static Color color();
};
