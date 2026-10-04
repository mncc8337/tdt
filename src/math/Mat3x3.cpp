#include "Mat3x3.h"
#include <cmath>

Mat3x3::Mat3x3() {
    m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
}

Vec3 Mat3x3::operator*(const Vec3& v) const {
    return Vec3(
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
    );
}

Mat3x3 Mat3x3::operator*(const Mat3x3& right) const {
    Mat3x3 res;
    res.m[0][0] = res.m[1][1] = res.m[2][2] = 0.0f;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                res.m[i][j] += m[i][k] * right.m[k][j];
            }
        }
    }
    return res;
}

Mat3x3 Mat3x3::transpose() const {
    Mat3x3 res;
    res.m[0][0] = m[0][0]; res.m[0][1] = m[1][0]; res.m[0][2] = m[2][0];
    res.m[1][0] = m[0][1]; res.m[1][1] = m[1][1]; res.m[1][2] = m[2][1];
    res.m[2][0] = m[0][2]; res.m[2][1] = m[1][2]; res.m[2][2] = m[2][2];
    return res;
}

Mat3x3 Mat3x3::rotateX(float angle) {
    Mat3x3 ret;
    float c = std::cos(angle);
    float s = std::sin(angle);
    ret.m[1][1] = c; ret.m[1][2] = -s;
    ret.m[2][1] = s; ret.m[2][2] = c;
    return ret;
}

Mat3x3 Mat3x3::rotateY(float angle) {
    Mat3x3 ret;
    float c = std::cos(angle);
    float s = std::sin(angle);
    ret.m[0][0] = c;  ret.m[0][2] = s;
    ret.m[2][0] = -s; ret.m[2][2] = c;
    return ret;
}

Mat3x3 Mat3x3::rotateZ(float angle) {
    Mat3x3 ret;
    float c = std::cos(angle);
    float s = std::sin(angle);
    ret.m[0][0] = c; ret.m[0][1] = -s;
    ret.m[1][0] = s; ret.m[1][1] = c;
    return ret;
}
