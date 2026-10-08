#pragma once

#include "Vec3.h"
#include "Vec2.h"
#include "Ray.h"

class Camera {
private:
    float focal_length;
    float exposure = 1.0f;

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

    float& getFocalLength();
    const float& getFocalLength() const;

    float& getExposure();
    const float& getExposure() const;

    const Vec3& getPosition() const;
    void setPosition(const Vec3 new_pos);

    const Vec3& getDirection() const;
    void setDirection(const Vec3 new_dir);

    void lookAt(const Vec3& v);

    Ray getRayAt(Vec2 uv) const;
};
