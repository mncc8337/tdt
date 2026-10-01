#pragma once

class Vec3{
protected:
    float x, y, z;

public:

    //tao vt
    Vec3();
    Vec3(float x, float y, float z);

    const float getX() const;
    const float getY() const;
    const float getZ() const;

    void setX(const float x);
    void setY(const float y);
    void setZ(const float z);

    // operator
    const Vec3 operator -() const;
    const Vec3 operator +(const Vec3& v) const;
    const Vec3& operator +=(const Vec3& v);
    const Vec3 operator -(const Vec3& v) const;
    const Vec3& operator -=(const Vec3& v);
    const Vec3 operator *(const float t) const;
    const Vec3& operator *=(const float x);
    const Vec3 operator /(const float t) const;
    const Vec3& operator /=(const float t);


    const float dot(const Vec3& other) const;
    const Vec3 cross(const Vec3& other) const;

    const float length_squared() const;
    const float length() const;

    const Vec3 normalized() const;
    const Vec3 normalize();
};
