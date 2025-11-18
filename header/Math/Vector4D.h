#pragma once
#pragma once
#include "pch.h"

class Vector4D
{
public:
    // デフォルトコンストラクタ（(0,0,0,0)に初期化）
    Vector4D() noexcept = default;

    // XYZW指定コンストラクタ
    Vector4D(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}

    // 同じ値でXYZW初期化（explicitで暗黙変換防止）
    explicit Vector4D(float v) noexcept : x(v), y(v), z(v), w(v) {}

    // コピーコンストラクタ
    Vector4D(const Vector4D&) noexcept = default;

    // 代入演算子
    Vector4D& operator=(const Vector4D&) noexcept = default;

    Vector4D operator+(const Vector4D& vec) const noexcept
    {
        return Vector4D(x + vec.x, y + vec.y, z + vec.z, w + vec.w);
    }

    Vector4D operator-(const Vector4D& vec) const noexcept
    {
        return Vector4D(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
    }

    Vector4D operator*(float scalar) const noexcept
    {
        return Vector4D(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    Vector4D& operator+=(const Vector4D& vec) noexcept
    {
        x += vec.x; y += vec.y; z += vec.z; w += vec.w;
        return *this;
    }

    Vector4D& operator-=(const Vector4D& vec) noexcept
    {
        x -= vec.x; y -= vec.y; z -= vec.z; w -= vec.w;
        return *this;
    }

    Vector4D& operator*=(float s) noexcept
    {
        x *= s; y *= s; z *= s; w *= s;
        return *this;
    }

    Vector4D& operator*=(const Vector4D& vec) noexcept
    {
        x *= vec.x; y *= vec.y; z *= vec.z; w *= vec.w;
        return *this;
    }

    Vector4D& operator/=(float s)
    {
        return *this *= (1.0f / s);
    }

    Vector4D& operator/=(const Vector4D& vec)
    {
        x /= vec.x; y /= vec.y; z /= vec.z; w /= vec.w;
        return *this;
    }

    float dot(const Vector4D& vec) const noexcept
    {
        return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
    }

    float length() const noexcept
    {
        return std::sqrt(dot(*this));
    }

    Vector4D GetSafeNormal() const noexcept
    {
        const float Size = length();
        return (Size > SMALL_NUMBER) ? (*this * (1.f / Size)) : Vector4D(0.f);
    }

public:
    float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
};
