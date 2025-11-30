#pragma once
#include "pch.h"
#include "Matrix4x4.h"
#include "Vector3D.h"
#include "Math/MathUtility.h"

class Quaternion
{
public:
    Quaternion() noexcept = default;
    constexpr Quaternion(float v_x, float v_y, float v_z, float v_w) noexcept : x(v_x), y(v_y), z(v_z), w(v_w) {}
    // explicit 意図せずにfloatがVector4D型に変更されないため
    explicit constexpr Quaternion(float v) noexcept : x(v), y(v), z(v), w(v) {}
    explicit constexpr Quaternion(const Vector3D& v) noexcept : x(v.x), y(v.y), z(v.z), w(1.0f) {}

    Quaternion& operator+=(const Quaternion& q) noexcept
    {
        x += q.x;
        y += q.y;
        z += q.z;
        w += q.w;
        return *this;
    }
    Quaternion& operator-=(const Quaternion& q) noexcept
    {
        x -= q.x;
        y -= q.y;
        z -= q.z;
        w -= q.w;
        return *this;
    }
    Quaternion& operator*=(float scale) noexcept
    {
        x *= scale;
        y *= scale;
        z *= scale;
        w *= scale;
        return *this;
    }
    Quaternion& operator*=(const Quaternion& q)
    {
        float x1 = x;
        float y1 = y;
        float z1 = z;
        float w1 = w;

        float x2 = q.x;
        float y2 = q.y;
        float z2 = q.z;
        float w2 = q.w;

        x = w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2;
        y = w1 * y2 + y1 * w2 + z1 * x2 - x1 * z2;
        z = w1 * z2 + z1 * w2 + x1 * y2 - y1 * x2;
        w = w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2;

        return *this;
    }

    float dot(const Quaternion& q) const
    {
        return w * q.w + x * q.x + y * q.y + z * q.z;
    }
    float length() const
    {
        return std::sqrt(dot(*this));
    }
    Quaternion& normalize()
    {
        float iLen = 1 / length();
        w *= iLen;
        x *= iLen;
        y *= iLen;
        z *= iLen;
        return *this;
    }

    static const Quaternion Euler(float x, float y, float z)
    {
        //float radX = Utility::DegreeToRadian(x);
        //float radY = Utility::DegreeToRadian(y);
        //float radZ = Utility::DegreeToRadian(z);

        //// 各軸の回転クォータニオンを計算
        //Quaternion qX = Quaternion::AngleAxis(-x, Vector3D(1.0f, 0.0f, 0.0f));
        //Quaternion qY = Quaternion::AngleAxis(-y, Vector3D(0.0f, 0.0f, 0.0f));
        //Quaternion qZ = Quaternion::AngleAxis(-z, Vector3D(0.0f, 0.0f, 1.0f));

        //return qZ * qX * qY;
    }

    static const Quaternion Euler(const Vector3D& vec)
    {
        //float radX = Utility::DegreeToRadian(vec.x);
        //float radY = Utility::DegreeToRadian(vec.y);
        //float radZ = Utility::DegreeToRadian(vec.z);

        //// 各軸の回転クォータニオンを計算
        //Quaternion qX = Quaternion::AngleAxis(-vec.x, Vector3D(1.0f, 0.0f, 0.0f));
        //Quaternion qY = Quaternion::AngleAxis(-vec.y, Vector3D(0.0f, 0.0f, 0.0f));
        //Quaternion qZ = Quaternion::AngleAxis(-vec.z, Vector3D(0.0f, 0.0f, 1.0f));

        //return qZ * qX * qY;
    }

    static const Vector3D EulerAngles(const Quaternion& q)
    {

    }

    //! @brief 指定した角度と回転軸からクォータニオンを生成
    //! @param[in] angle 角度
    //! @param[in] axis 回転軸
    //! @return クォータニオン
    static const Quaternion AngleAxis(float angle, const Vector3D& axis)
    {
        Vector3D normalizedAxis = axis.GetSafeNormal();
        float radian = MathUtility::DegreeToRadian(angle);

        // 半角の値を計算
        float halfAngle = radian * 0.5f;
        float sinHalfAngle = sin(halfAngle);
        float cosHalfAngle = cos(halfAngle);

        return Quaternion(
            normalizedAxis.x * sinHalfAngle,
            normalizedAxis.y * sinHalfAngle,
            normalizedAxis.z * sinHalfAngle,
            cosHalfAngle
        );
    }

public:
    float x, y, z, w;
};

inline constexpr const Quaternion operator*(const Quaternion& vec1, const Quaternion& vec2) noexcept;

inline constexpr const Quaternion operator*(const Quaternion& vec1, const Quaternion& vec2) noexcept
{
    float w1 = vec1.w;
    float x1 = vec1.x;
    float y1 = vec1.y;
    float z1 = vec1.z;

    float w2 = vec2.w;
    float x2 = vec2.x;
    float y2 = vec2.y;
    float z2 = vec2.z;

    return Quaternion(
        w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2,
        w1 * y2 + y1 * w2 + z1 * x2 - x1 * z2,
        w1 * z2 + z1 * w2 + x1 * y2 - y1 * x2,
        w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2
    );
}