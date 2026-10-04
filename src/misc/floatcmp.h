#define EPSILON float(1e-8)
#define RAY_ORIGIN_OFFSET float(1e-6)
#define FAR_DISTANCE float(1e9)

#include <cstdlib>

inline bool fequal(const float a, const float b) {
    return std::abs(a - b) < EPSILON;
}

inline bool fequal0(const float t) {
    return std::abs(t) < EPSILON;
}
