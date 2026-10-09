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

Vec3 RNG::direction() {
    return Vec3(normal(), normal(), normal()).normalized();
}

Vec3 RNG::directionFast() {
    Vec3 p;
    do {
        p = Vec3(uniform(-1, 1), uniform(-1, 1), uniform(-1, 1));
    } while (p.length_squared() > 1.0f);
    return p;
}

Color RNG::color() {
    return Vec3(uniform(), uniform(), uniform());
}
