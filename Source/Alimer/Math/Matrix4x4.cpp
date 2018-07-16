//
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Math/Matrix4x4.h"
#include "../Math/MathUtil.h"

#if ALIMER_SSE2
#include <DirectXMath.h>
using namespace DirectX;

// Vector3 and XMFLOAT3
static_assert(sizeof(XMFLOAT3) == sizeof(Alimer::Vector3), "XMFLOAT3 and Vector3 mismatch");
static_assert(offsetof(Alimer::Vector3, x) == offsetof(XMFLOAT3, x), "XMFLOAT3 and Vector3 x mismatch");
static_assert(offsetof(Alimer::Vector3, y) == offsetof(XMFLOAT3, y), "XMFLOAT3 and Vector3 y mismatch");
static_assert(offsetof(Alimer::Vector3, z) == offsetof(XMFLOAT3, z), "XMFLOAT3 and Vector3 z mismatch");

// Matrix4x4 and XMFLOAT4X4
static_assert(sizeof(XMFLOAT4X4) == sizeof(Alimer::Matrix4x4), "XMFLOAT4X4 and Matrix4x4 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _11) == offsetof(XMFLOAT4X4, _11), "XMFLOAT4X4 and Matrix4x4 _11 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _12) == offsetof(XMFLOAT4X4, _12), "XMFLOAT4X4 and Matrix4x4 _12 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _13) == offsetof(XMFLOAT4X4, _13), "XMFLOAT4X4 and Matrix4x4 _13 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _14) == offsetof(XMFLOAT4X4, _14), "XMFLOAT4X4 and Matrix4x4 _14 mismatch");

static_assert(offsetof(Alimer::Matrix4x4, _21) == offsetof(XMFLOAT4X4, _21), "XMFLOAT4X4 and Matrix4x4 _21 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _22) == offsetof(XMFLOAT4X4, _22), "XMFLOAT4X4 and Matrix4x4 _22 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _23) == offsetof(XMFLOAT4X4, _23), "XMFLOAT4X4 and Matrix4x4 _23 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _24) == offsetof(XMFLOAT4X4, _24), "XMFLOAT4X4 and Matrix4x4 _24 mismatch");

static_assert(offsetof(Alimer::Matrix4x4, _31) == offsetof(XMFLOAT4X4, _31), "XMFLOAT4X4 and Matrix4x4 _31 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _32) == offsetof(XMFLOAT4X4, _32), "XMFLOAT4X4 and Matrix4x4 _32 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _33) == offsetof(XMFLOAT4X4, _33), "XMFLOAT4X4 and Matrix4x4 _33 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _34) == offsetof(XMFLOAT4X4, _34), "XMFLOAT4X4 and Matrix4x4 _34 mismatch");

static_assert(offsetof(Alimer::Matrix4x4, _41) == offsetof(XMFLOAT4X4, _41), "XMFLOAT4X4 and Matrix4x4 _41 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _42) == offsetof(XMFLOAT4X4, _42), "XMFLOAT4X4 and Matrix4x4 _42 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _43) == offsetof(XMFLOAT4X4, _43), "XMFLOAT4X4 and Matrix4x4 _43 mismatch");
static_assert(offsetof(Alimer::Matrix4x4, _44) == offsetof(XMFLOAT4X4, _44), "XMFLOAT4X4 and Matrix4x4 _44 mismatch");

inline XMVECTOR XM_CALLCONV XMVECTORFromAlimer(const Alimer::Vector3& source)
{
#if defined(_XM_NO_INTRINSICS_)
    XMVECTOR V;
    V.vector4_f32[0] = source.x;
    V.vector4_f32[1] = source.y;
    V.vector4_f32[2] = source.z;
    V.vector4_f32[3] = 0.f;
    return V;
#elif defined(_XM_ARM_NEON_INTRINSICS_)
    float32x2_t x = vld1_f32(reinterpret_cast<const float*>(pSource));
    float32x2_t zero = vdup_n_f32(0);
    float32x2_t y = vld1_lane_f32(reinterpret_cast<const float*>(pSource) + 2, zero, 0);
    return vcombine_f32(x, y);
#elif defined(_XM_SSE_INTRINSICS_)
    __m128 x = _mm_load_ss(&source.x);
    __m128 y = _mm_load_ss(&source.y);
    __m128 z = _mm_load_ss(&source.z);
    __m128 xy = _mm_unpacklo_ps(x, y);
    return _mm_movelh_ps(xy, z);
#endif
}

#endif

namespace Alimer
{
    const Matrix4x4 Matrix4x4::Identity = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };

    Matrix4x4::Matrix4x4(const float* pArray)
    {
        assert(pArray != nullptr);

        m[0][0] = pArray[0];
        m[0][1] = pArray[1];
        m[0][2] = pArray[2];
        m[0][3] = pArray[3];

        m[1][0] = pArray[4];
        m[1][1] = pArray[5];
        m[1][2] = pArray[6];
        m[1][3] = pArray[7];

        m[2][0] = pArray[8];
        m[2][1] = pArray[9];
        m[2][2] = pArray[10];
        m[2][3] = pArray[11];

        m[3][0] = pArray[12];
        m[3][1] = pArray[13];
        m[3][2] = pArray[14];
        m[3][3] = pArray[15];
    }

    void Matrix4x4::CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance, Matrix4x4* result)
    {
        ALIMER_ASSERT(result);

        assert(nearPlaneDistance > 0.f && farPlaneDistance > 0.f);
        assert(!ScalarNearEqual(fieldOfView, 0.0f, 0.00001f * 2.0f));
        assert(!ScalarNearEqual(aspectRatio, 0.0f, 0.00001f));
        assert(!ScalarNearEqual(farPlaneDistance, nearPlaneDistance, 0.00001f));

#if ALIMER_SSE2
        XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(result),
            XMMatrixPerspectiveFovRH(fieldOfView, aspectRatio, fieldOfView, farPlaneDistance)
        );
#else
        float sinFov;
        float cosFov;
        SinCos(0.5f * fieldOfView, &sinFov, &cosFov);

        float Height = cosFov / sinFov;
        float Width = Height / aspectRatio;
        float range = farPlaneDistance / (nearPlaneDistance - farPlaneDistance);

        result->m[0][0] = Width;
        result->m[0][1] = 0.0f;
        result->m[0][2] = 0.0f;
        result->m[0][3] = 0.0f;

        result->m[1][0] = 0.0f;
        result->m[1][1] = Height;
        result->m[1][2] = 0.0f;
        result->m[1][3] = 0.0f;

        result->m[2][0] = 0.0f;
        result->m[2][1] = 0.0f;
        result->m[2][2] = range;
        result->m[2][3] = -1.0f;

        result->m[3][0] = 0.0f;
        result->m[3][1] = 0.0f;
        result->m[3][2] = range * nearPlaneDistance;
        result->m[3][3] = 0.0f;
#endif
    }

    Matrix4x4 Matrix4x4::CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance)
    {
        Matrix4x4 result;
        CreatePerspectiveFieldOfView(fieldOfView, aspectRatio, nearPlaneDistance, farPlaneDistance, &result);
        return result;
    }

    void Matrix4x4::CreateLookAt(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector, Matrix4x4* result)
    {
        ALIMER_ASSERT(result);

#if ALIMER_SSE2
        XMVECTOR simdCameraPosition = XMVECTORFromAlimer(cameraPosition);
        XMVECTOR simdCameraTarget = XMVECTORFromAlimer(cameraTarget);
        XMVECTOR simdCameraUpVector = XMVECTORFromAlimer(cameraUpVector);

        XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(result),
            XMMatrixLookAtRH(simdCameraPosition, simdCameraTarget, simdCameraUpVector)
        );
#else
        Vector3 zaxis = cameraPosition - cameraTarget; zaxis.Normalize();
        Vector3 xaxis = Vector3::Cross(cameraUpVector, zaxis); xaxis.Normalize();
        Vector3 yaxis;
        Vector3::Cross(zaxis, xaxis, &yaxis);

        result->_11 = xaxis.x;
        result->_12 = yaxis.x;
        result->_13 = zaxis.x;
        result->_14 = 0.0f;
        result->_21 = xaxis.y;
        result->_22 = yaxis.y;
        result->_23 = zaxis.y;
        result->_24 = 0.0f;
        result->_31 = xaxis.z;
        result->_32 = yaxis.z;
        result->_33 = zaxis.z;
        result->_34 = 0.0f;
        result->_41 = -Vector3::Dot(xaxis, cameraPosition);
        result->_42 = -Vector3::Dot(yaxis, cameraPosition);
        result->_43 = -Vector3::Dot(zaxis, cameraPosition);
        result->_44 = 1.0f;
#endif
    }

    Matrix4x4 Matrix4x4::CreateLookAt(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector)
    {
        Matrix4x4 result;
        CreateLookAt(cameraPosition, cameraTarget, cameraUpVector, &result);
        return result;
    }
    }
