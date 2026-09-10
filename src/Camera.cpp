#include "Camera.h"

Camera::Camera(
    unsigned viewport_width,
    unsigned viewport_height,
    double focal_length,
    Vec3 position,
    Vec3 direction
):
    viewport_width(viewport_width),
    viewport_height(viewport_height),
    focal_length(focal_length),
    position(position),
    direction(direction) {}

const Ray Camera::getRayAt(int px, int py) const {
    Ray ray(position, direction);

    // TODO: calculate direction
    return ray;
}

const unsigned& Camera::getViewportWidth() const {
    return viewport_width;
}

void Camera::setViewportWidth(const unsigned new_width) {
    viewport_width = new_width;
}

const unsigned& Camera::getViewportHeight() const {
    return viewport_height;
}

void Camera::setViewportHeight(const unsigned new_height) {
    viewport_height = new_height;
}

const double& Camera::getFocalLength() const {
    return focal_length;
}

void Camera::getFocalLength(const double new_fl) {
    focal_length = new_fl;
}

const Vec3& Camera::getPosition() const {
    return position;
}

void Camera::setPosition(const Vec3 new_pos) {
    position = new_pos;
}

const Vec3& Camera::getDirection() const {
    return direction;
}

void Camera::setDirection(const Vec3 new_dir) {
    direction = new_dir;
}
