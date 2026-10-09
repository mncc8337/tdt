#include "Camera.h"
#include "floatcmp.h"
#include "misc/RNG.h"

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

Ray Camera::getRayAt(Vec2 uv, bool jitter) const {
    Ray ray;
    ray.origin = position;
    ray.direction = focal_length * w + uv.x * u + uv.y * v;

    if(jitter) {
        if(!fequal0(aperture)) {
            // offset ray origin for defocus effect
            Vec2 defocus_jitter = RNG::pointInCircle(aperture / 2.0f);
            ray.origin += u * defocus_jitter.x + v * defocus_jitter.y;
        }
        if(!fequal0(diverge_strength)) {
            // offset viewpoint for anti-aliasing
            Vec2 jitter = RNG::pointInCircle(diverge_strength);
            ray.direction += u * jitter.x + v * jitter.y;
        }
    }

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


float& Camera::getAperture() {
    return aperture;
}

const float& Camera::getAperture() const {
    return aperture;
}

float& Camera::getDivergeStrength() {
    return diverge_strength;
}

const float& Camera::getDivergeStrength() const {
    return diverge_strength;
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
