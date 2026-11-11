#include "RNG.h"
#include <atomic>
#include <cstdint>
#include <numbers>

// Each thread starts from a distinct, well-mixed seed. The counter is atomic
// because several threads can touch the RNG for the first time concurrently.
static std::atomic<std::uint32_t> seed_counter{0};

// splitmix32 mixing step (Weyl increment + xor-shift-multiply avalanche)
// turn the thread counter into a seed.
static std::uint32_t mixSeed(std::uint32_t x) {
    x += 0x9E3779B9u;
    x = (x ^ (x >> 16)) * 0x21F0AAADu;
    x = (x ^ (x >> 15)) * 0x735A2D97u;
    return x ^ (x >> 15);
}

thread_local std::uniform_real_distribution<float> RNG::uniform_dist(0, 1);
thread_local std::normal_distribution<float> RNG::normal_dist(0, 1);
thread_local std::mt19937 RNG::generator(
    mixSeed(0x51C3A7u + seed_counter.fetch_add(1, std::memory_order_relaxed))
);

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

Vec2 RNG::pointInCircle(const float radius) {
    float angle = uniform() * 2.0f * std::numbers::pi_v<float>;
    Vec2 point_on_circle(cos(angle) * radius, sin(angle) * radius);
    return point_on_circle * sqrt(uniform() * radius);
}

Vec3 RNG::vec3() {
    return Vec3(uniform(), uniform(), uniform());
}

Vec2 RNG::vec2() {
    return Vec2(uniform());
}
