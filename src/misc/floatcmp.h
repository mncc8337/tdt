#pragma once

#define EPSILON float(1e-6)
#define RAY_ORIGIN_OFFSET float(1e-5)
#define FAR_DISTANCE float(1e9)

#include <cstdlib>

inline bool fequal(const float& a, const float& b, const float epsilon = EPSILON) {
    return std::abs(a - b) < epsilon;
}

inline bool fequal0(const float& t, const float epsilon = EPSILON) {
    return std::abs(t) < epsilon;
}
