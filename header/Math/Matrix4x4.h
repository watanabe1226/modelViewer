#pragma once
#include "pch.h"
#include "Vector3D.h"
#include "Vector4D.h"
//#include "Quaternion.h"

class Matrix4x4
{
public:
    // コンストラクタ
    Matrix4x4() noexcept
    {
        setIdentity();
    }

    void setIdentity() noexcept
    {
        ::memset(m_mat, 0, sizeof(float) * 16);
        m_mat[0][0] = 1.f;
        m_mat[1][1] = 1.f;
        m_mat[2][2] = 1.f;
        m_mat[3][3] = 1.f;
    }

    static Matrix4x4 Identity() noexcept
    {
        Matrix4x4 mat;
        mat.setIdentity();
        return mat;
    }

    //// =======================
    //// FbxAMatrix 変換
    //// =======================
    //static Matrix4x4 ConvertMatrix(const FbxAMatrix& fbmat) noexcept
    //{
    //    Matrix4x4 mat;
    //    for (int row = 0; row < 4; ++row)
    //    {
    //        for (int col = 0; col < 4; ++col)
    //            mat.m_mat[row][col] = static_cast<float>(fbmat.Get(row, col));
    //    }
    //    return mat;
    //}

    // =======================
    // 回転行列設定
    // =======================
    void setRotationX(float x) noexcept
    {
        float c = std::cos(x);
        float s = std::sin(x);
        m_mat[1][1] = c;  m_mat[1][2] = s;
        m_mat[2][1] = -s; m_mat[2][2] = c;
    }

    void setRotationY(float y) noexcept
    {
        float c = std::cos(y);
        float s = std::sin(y);
        m_mat[0][0] = c;  m_mat[0][2] = -s;
        m_mat[2][0] = s;  m_mat[2][2] = c;
    }

    void setRotationZ(float z) noexcept
    {
        float c = std::cos(z);
        float s = std::sin(z);
        m_mat[0][0] = c;  m_mat[0][1] = s;
        m_mat[1][0] = -s; m_mat[1][1] = c;
    }

    void operator*=(const Matrix4x4& mat) noexcept
    {
        *this = *this * mat;
    }

    Matrix4x4 operator*(const Matrix4x4& mat) const noexcept
    {
        Matrix4x4 out;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                out.m_mat[i][j] =
                    m_mat[i][0] * mat.m_mat[0][j] +
                    m_mat[i][1] * mat.m_mat[1][j] +
                    m_mat[i][2] * mat.m_mat[2][j] +
                    m_mat[i][3] * mat.m_mat[3][j];
            }
        }
        return out;
    }

    void setMatrix(const Matrix4x4& mat) noexcept
    {
        ::memcpy(m_mat, mat.m_mat, sizeof(float) * 16);
    }

    // =======================
    // 行列からベクトル変換
    // =======================
    static Vector3D Apply(const Matrix4x4& matrix, const Vector3D& vec) noexcept
    {
        const Vector4D row0(matrix.m_mat[0][0], matrix.m_mat[0][1], matrix.m_mat[0][2], matrix.m_mat[0][3]);
        const Vector4D row1(matrix.m_mat[1][0], matrix.m_mat[1][1], matrix.m_mat[1][2], matrix.m_mat[1][3]);
        const Vector4D row2(matrix.m_mat[2][0], matrix.m_mat[2][1], matrix.m_mat[2][2], matrix.m_mat[2][3]);

        float x = vec.x * row0.x + vec.y * row0.y + vec.z * row0.z + row0.w;
        float y = vec.x * row1.x + vec.y * row1.y + vec.z * row1.z + row1.w;
        float z = vec.x * row2.x + vec.y * row2.y + vec.z * row2.z + row2.w;

        return Vector3D(x, y, z);
    }

    // =======================
    // ビュー・投影・変換行列
    // =======================
    static Matrix4x4 setLookAtLH(const Vector3D& pos, const Vector3D& target, const Vector3D& up) noexcept
    {
        Matrix4x4 mat;

        Vector3D zaxis = (target - pos).GetSafeNormal();
        Vector3D xaxis = up.cross(zaxis).GetSafeNormal();
        Vector3D yaxis = zaxis.cross(xaxis);

        mat.m_mat[0][0] = xaxis.x; mat.m_mat[0][1] = yaxis.x; mat.m_mat[0][2] = zaxis.x; mat.m_mat[0][3] = 0.f;
        mat.m_mat[1][0] = xaxis.y; mat.m_mat[1][1] = yaxis.y; mat.m_mat[1][2] = zaxis.y; mat.m_mat[1][3] = 0.f;
        mat.m_mat[2][0] = xaxis.z; mat.m_mat[2][1] = yaxis.z; mat.m_mat[2][2] = zaxis.z; mat.m_mat[2][3] = 0.f;
        mat.m_mat[3][0] = -xaxis.dot(pos);
        mat.m_mat[3][1] = -yaxis.dot(pos);
        mat.m_mat[3][2] = -zaxis.dot(pos);
        mat.m_mat[3][3] = 1.f;

        return mat;
    }

    static Matrix4x4 setPerspectiveFovLH(float fov, float aspect, float znear, float zfar) noexcept
    {
        Matrix4x4 mat;
        float yscale = 1.f / std::tan(fov * 0.5f);
        float xscale = yscale / aspect;

        mat.m_mat[0][0] = xscale; mat.m_mat[0][1] = 0.f;     mat.m_mat[0][2] = 0.f;                mat.m_mat[0][3] = 0.f;
        mat.m_mat[1][0] = 0.f;    mat.m_mat[1][1] = yscale;  mat.m_mat[1][2] = 0.f;                mat.m_mat[1][3] = 0.f;
        mat.m_mat[2][0] = 0.f;    mat.m_mat[2][1] = 0.f;     mat.m_mat[2][2] = zfar / (zfar - znear); mat.m_mat[2][3] = 1.f;
        mat.m_mat[3][0] = 0.f;    mat.m_mat[3][1] = 0.f;     mat.m_mat[3][2] = -znear * zfar / (zfar - znear); mat.m_mat[3][3] = 0.f;

        return mat;
    }

    static Matrix4x4 setOrthoLH(float width, float height, float near_plane, float far_plane) noexcept
    {
        Matrix4x4 mat;
        float fRange = 1.f / (far_plane - near_plane);

        mat.m_mat[0][0] = 2.f / width; mat.m_mat[0][1] = 0.f;         mat.m_mat[0][2] = 0.f;        mat.m_mat[0][3] = 0.f;
        mat.m_mat[1][0] = 0.f;         mat.m_mat[1][1] = 2.f / height; mat.m_mat[1][2] = 0.f;        mat.m_mat[1][3] = 0.f;
        mat.m_mat[2][0] = 0.f;         mat.m_mat[2][1] = 0.f;         mat.m_mat[2][2] = fRange;    mat.m_mat[2][3] = 0.f;
        mat.m_mat[3][0] = 0.f;         mat.m_mat[3][1] = 0.f;         mat.m_mat[3][2] = -fRange * near_plane; mat.m_mat[3][3] = 1.f;

        return mat;
    }

    static Matrix4x4 setOrthoOffsetLH(float left, float right, float bottom, float top, float nearZ, float farZ) noexcept
    {
        Matrix4x4 mat;
        float rWidth = 1.f / (right - left);
        float rHeight = 1.f / (top - bottom);
        float fRange = 1.f / (farZ - nearZ);

        mat.m_mat[0][0] = 2.f * rWidth; mat.m_mat[0][1] = 0.f;       mat.m_mat[0][2] = 0.f;    mat.m_mat[0][3] = 0.f;
        mat.m_mat[1][0] = 0.f;          mat.m_mat[1][1] = 2.f * rHeight; mat.m_mat[1][2] = 0.f;    mat.m_mat[1][3] = 0.f;
        mat.m_mat[2][0] = 0.f;          mat.m_mat[2][1] = 0.f;       mat.m_mat[2][2] = fRange; mat.m_mat[2][3] = 0.f;
        mat.m_mat[3][0] = -(left + right) * rWidth;
        mat.m_mat[3][1] = -(top + bottom) * rHeight;
        mat.m_mat[3][2] = -nearZ * fRange;
        mat.m_mat[3][3] = 1.f;

        return mat;
    }

    //// クォータニオンから回転行列
    //static Matrix4x4 QuaternionToMatrix(const Quaternion& q) noexcept
    //{
    //    Matrix4x4 mat;
    //    float w = q.w, x = q.x, y = q.y, z = q.z;

    //    mat.m_mat[0][0] = 1.f - 2.f * (y * y + z * z);
    //    mat.m_mat[0][1] = 2.f * (x * y - z * w);
    //    mat.m_mat[0][2] = 2.f * (x * z + y * w);
    //    mat.m_mat[0][3] = 0.f;

    //    mat.m_mat[1][0] = 2.f * (x * y + z * w);
    //    mat.m_mat[1][1] = 1.f - 2.f * (x * x + z * z);
    //    mat.m_mat[1][2] = 2.f * (y * z - x * w);
    //    mat.m_mat[1][3] = 0.f;

    //    mat.m_mat[2][0] = 2.f * (x * z - y * w);
    //    mat.m_mat[2][1] = 2.f * (y * z + x * w);
    //    mat.m_mat[2][2] = 1.f - 2.f * (x * x + y * y);
    //    mat.m_mat[2][3] = 0.f;

    //    mat.m_mat[3][0] = 0.f;
    //    mat.m_mat[3][1] = 0.f;
    //    mat.m_mat[3][2] = 0.f;
    //    mat.m_mat[3][3] = 1.f;

    //    return mat;
    //}

    // 平行移動行列
    static Matrix4x4 TransitionToMatrix(const Vector3D& trans) noexcept
    {
        Matrix4x4 mat;
        mat.setIdentity();
        mat.m_mat[3][0] = trans.x;
        mat.m_mat[3][1] = trans.y;
        mat.m_mat[3][2] = trans.z;
        return mat;
    }

    // スケーリング行列
    static Matrix4x4 ScalingToMatrix(const Vector3D& scale) noexcept
    {
        Matrix4x4 mat;
        mat.setIdentity();
        mat.m_mat[0][0] = scale.x;
        mat.m_mat[1][1] = scale.y;
        mat.m_mat[2][2] = scale.z;
        return mat;
    }


public:
    float m_mat[4][4] = {};
};
