#pragma once

#include "Vec3.h"

class Ray {
private:
    Vec3 origin;
    Vec3 direction;

public:
    Ray(Vec3 origin, Vec3 direction);

    const Vec3& getOrigin() const;
    void setOrigin(const Vec3 new_org);

    const Vec3& getDirection() const;
    void setDirection(const Vec3 new_dir);

    Vec3 point(const double distance) const;
};
