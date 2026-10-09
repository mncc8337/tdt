#pragma once

#include <random>
#include "math/Vec3.h"
#include "math/Vec2.h"

class RNG {
private:
    static std::uniform_real_distribution<float> uniform_dist;
    static std::normal_distribution<float> normal_dist;
    static std::mt19937 generator;

public:
    static float uniform();
    static float uniform(float from, float to);
    static float normal();
    static Vec3 direction();
    static Vec3 directionFast();
    static Vec2 pointInCircle(const float radius = 1.0f);
    static Color color();
};
