#include "Ray.h"

Ray::Ray(Vec3 origin, Vec3 direction):
    origin(origin),
    direction(direction) {}

const Vec3& Ray::getOrigin() const {
    return origin;
}

void Ray::setOrigin(const Vec3 new_org) {
    origin = new_org;
}

const Vec3& Ray::getDirection() const {
    return origin;
}

void Ray::setDirection(const Vec3 new_dir) {
    direction = new_dir;
}

Vec3 Ray::point(const double distance) const {
    return origin + direction * distance;
}
