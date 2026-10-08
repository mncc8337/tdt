#include "Camera.h"

Camera::Camera(
    float focal_length,
    Vec3 position,
    Vec3 lookat
):
    focal_length(focal_length),
    position(position),
    lookat(lookat) {
    computeOrientation();
}

void Camera::computeOrientation() {
    w = (lookat - position).normalized();
    u = Vec3(0, 1, 0).cross(w);
    v = w.cross(u);
}

Ray Camera::getRayAt(Vec2 uv) const {
    Ray ray;
    ray.origin = position;
    ray.direction = focal_length * w + uv.x * u + uv.y * v;
    ray.direction.normalize();
    return ray;
}

float& Camera::getFocalLength() {
    return focal_length;
}

const float& Camera::getFocalLength() const {
    return focal_length;
}

float& Camera::getExposure() {
    return exposure;
}

const float& Camera::getExposure() const {
    return exposure;
}

const Vec3& Camera::getPosition() const {
    return position;
}

void Camera::setPosition(const Vec3 new_pos) {
    position = new_pos;
}

const Vec3& Camera::getDirection() const {
    return lookat;
}

void Camera::setDirection(const Vec3 new_dir) {
    lookat = position + new_dir;
    computeOrientation();
}

void Camera::lookAt(const Vec3& v) {
    lookat = v;
    computeOrientation();
}
