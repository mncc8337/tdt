#pragma once

#include "Vec3.h"
#include "Ray.h"

class Camera {
private:
    unsigned viewport_width;
    unsigned viewport_height;
    double focal_length;

    Vec3 position;
    Vec3 direction;

public:
    Camera(
        unsigned viewport_width,
        unsigned viewport_height,
        double focal_length,
        Vec3 position,
        Vec3 direction
    );

    const unsigned& getViewportWidth() const;
    void setViewportWidth(const unsigned new_width);

    const unsigned& getViewportHeight() const;
    void setViewportHeight(const unsigned new_height);

    const double& getFocalLength() const;
    void getFocalLength(const double new_fl);

    const Vec3& getPosition() const;
    void setPosition(const Vec3 new_pos);

    const Vec3& getDirection() const;
    void setDirection(const Vec3 new_dir);

    const Ray getRayAt(int px, int py) const;
};
