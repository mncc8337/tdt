#pragma once

class Vec3{
    public:
        double x, y, z;

        //tao vt
        Vec3();
        Vec3(double x, double y, double z);

        // operator
        Vec3 operator+(const Vec3& other) const;
        Vec3 operator-(const Vec3& other) const;
        Vec3 operator*(double t) const;
        Vec3 operator/(double t) const;

        double VoHuong(const Vec3& other) const;
        Vec3 CoHuong(const Vec3& other) const;

        double length() const;
        Vec3 normal() const; // chuan hoa 
};
