#pragma once

class Vec3{
protected:
    float x, y, z;

public:

    //tao vt
    Vec3();
    Vec3(float x, float y, float z);

    float getX() const;
    float getY() const;
    float getZ() const;

    void setX(const float x);
    void setY(const float y);
    void setZ(const float z);

    Vec3 operator -() const;

    Vec3 operator +(const Vec3& v) const;
    const Vec3& operator +=(const Vec3& v);

    Vec3 operator -(const Vec3& v) const;
    const Vec3& operator -=(const Vec3& v);

    Vec3 operator *(const float t) const;
    const Vec3& operator *=(const float x);
    friend Vec3 operator *(const float t, const Vec3& v);

    Vec3 operator *(const Vec3& v) const;
    Vec3 operator *=(const Vec3& v);

    Vec3 operator /(const float t) const;
    const Vec3& operator /=(const float t);


    float dot(const Vec3& other) const;
    Vec3 cross(const Vec3& other) const;

    float length_squared() const;
    float length() const;

    Vec3 normalized() const;
    Vec3 normalize();

    Vec3 lerp(const Vec3& v, const float t) const;

    Vec3 reflection(Vec3 n) const;
};
