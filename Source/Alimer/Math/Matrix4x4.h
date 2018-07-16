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

#pragma once

#include "../Math/Vector3.h"
#include "../Math/Vector4.h"

namespace Alimer
{
    class Plane;
    class Quaternion;

    /// Defines a 4 x 4 floating point matrix representing a 3D transformation.
    class ALIMER_API Matrix4x4
    {
    public:
        union
        {
            struct
            {
                float _11, _12, _13, _14;
                float _21, _22, _23, _24;
                float _31, _32, _33, _34;
                float _41, _42, _43, _44;
            };
            float m[4][4];
        };

        Matrix4x4() = default;

        Matrix4x4(const Matrix4x4&) = default;
        Matrix4x4& operator=(const Matrix4x4&) = default;

        Matrix4x4(Matrix4x4&&) = default;
        Matrix4x4& operator=(Matrix4x4&&) = default;

        constexpr Matrix4x4(
            float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33)
            : _11(m00), _12(m01), _13(m02), _14(m03)
            , _21(m10), _22(m11), _23(m12), _24(m13)
            , _31(m20), _32(m21), _33(m22), _34(m23)
            , _41(m30), _42(m31), _43(m32), _44(m33)
        {
        }

        explicit Matrix4x4(const float *pArray);

        /// Return float data.
        const float* Data() const { return &_11; }

        // Comparison operators
        bool operator == (const Matrix4x4& rhs)  const
        {
            const float* leftData = Data();
            const float* rightData = rhs.Data();

            for (unsigned i = 0; i < 16; ++i)
            {
                if (leftData[i] != rightData[i])
                    return false;
            }

            return true;
        }

        bool operator != (const Matrix4x4& rhs) const { return !(*this == rhs); }

        float operator() (size_t row, size_t column) const { return m[row][column]; }
        float& operator() (size_t row, size_t column) { return m[row][column]; }

        static void CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance, Matrix4x4* result);
        static Matrix4x4 CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance);

        static void CreateLookAt(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector, Matrix4x4* result);
        static Matrix4x4 CreateLookAt(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector);

        // Constants
        static const Matrix4x4 Identity;
    };

    using mat4 = Matrix4x4;
}

