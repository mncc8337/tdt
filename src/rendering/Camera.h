#pragma once

#include "Vec3.h"
#include "Vec2.h"
#include "Ray.h"

class Camera {
private:
    float focal_length;

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
    void getFocalLength(const float new_fl);

    const Vec3& getPosition() const;
    void setPosition(const Vec3 new_pos);

    const Vec3& getDirection() const;
    void setDirection(const Vec3 new_dir);

    void lookAt(const Vec3& v);

    const Ray getRayAt(Vec2 uv) const;
};
