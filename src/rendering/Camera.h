#pragma once

#include "Vec3.h"
#include "Ray.h"

class Camera {
private:
    float focal_length;

    Vec3 position;
    Vec3 lookat;

public:
    Camera(
        float focal_length,
        Vec3 position,
        Vec3 direction
    );

    const float& getFocalLength() const;
    void getFocalLength(const float new_fl);

    const Vec3& getPosition() const;
    void setPosition(const Vec3 new_pos);

    const Vec3& getDirection() const;
    void setDirection(const Vec3 new_dir);

    const Ray getRayAt(float px, float py) const;
};
