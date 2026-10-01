#include "Camera.h"

Camera::Camera(
    float focal_length,
    Vec3 position,
    Vec3 lookat
):
    focal_length(focal_length),
    position(position),
    lookat(lookat) {}

const Ray Camera::getRayAt(float px, float py) const {
    // TODO: rotate ray_direction according to camera.lookat
    Vec3 ray_direction = Vec3(px, py, -focal_length);
    ray_direction.normalize();

    return Ray(position, ray_direction);
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
    lookat = new_dir;
}
