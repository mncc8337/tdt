#include "Ray.h"

Ray::Ray(Vec3 origin, Vec3 direction):
    origin(origin),
    direction(direction) {}

Vec3 Ray::point(const float distance) const {
    return origin + direction * distance;
}
