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

const Ray Camera::getRayAt(Vec2 uv) const {
    Ray ray;
    ray.origin = position;
    ray.direction = focal_length * w + uv.x * u + uv.y * v;
    return ray;
}

const float& Camera::getFocalLength() const {
    return focal_length;
}

void Camera::getFocalLength(const float new_fl) {
    focal_length = new_fl;
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
