#include "RNG.h"

std::uniform_real_distribution<float> RNG::uniform_dist(0, 1);
std::normal_distribution<float> RNG::normal_dist(0, 1);
std::mt19937 RNG::generator;

float RNG::uniform() {
    return uniform_dist(generator);
}

float RNG::uniform(float from, float to) {
    return uniform_dist(generator) * (to - from) + from;
}

float RNG::normal() {
    return normal_dist(generator);
}

Vec3 RNG::direction_unnormalized() {
    return Vec3(normal(), normal(), normal());
}

Vec3 RNG::direction_normalized() {
    return direction_unnormalized().normalized();
}

Vec3 RNG::direction_in_unit_sphere() {
    while(true) {
        Vec3 v(uniform() * 2.0f - 1.0f, uniform() * 2.0f - 1.0f, uniform() * 2.0f - 1.0f);
        if(v.length_squared() < 1.0f) return v;
    }
}

Color RNG::color() {
    return Vec3(uniform(), uniform(), uniform());
}
