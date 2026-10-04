#define EPSILON 1e-6

#include <cstdlib>

inline bool fequal(const float a, const float b) {
    return std::abs(a - b) < EPSILON;
}

inline bool fequal0(const float t) {
    return std::abs(t) < EPSILON;
}
