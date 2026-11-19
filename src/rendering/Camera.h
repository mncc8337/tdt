#pragma once

#include "Vec3.h"
#include "Vec2.h"
#include "Ray.h"

class Camera {
private:
    float focal_length;
    float aperture = 0.02f;

    Vec3 position;
    Vec3 lookat;
    Vec3 w, u, v;

    void computeOrientation();

public:
    Camera(
        float focal_length,
        Vec3 position,
        Vec3 lookat
    );

    const float& getFocalLength() const;
    void setFocalLength(const float f);

    const float& getAperture() const;
    void setAperture(const float a);

    const Vec3& getPosition() const;
    void setPosition(const Vec3 new_pos);

    const Vec3& getDirection() const;
    void setDirection(const Vec3 new_dir);

    const Vec3& getRight() const;
    const Vec3& getUp() const;

    void lookAt(const Vec3& l);

    Ray getRayAt(Vec2 uv, bool jitter = true) const;
};
