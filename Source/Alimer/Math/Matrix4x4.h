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

#include "../Base/String.h"
#include "../Math/Math.h"
#include "../Math/MathUtil.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201 4203 4244 4702)
#endif

namespace Alimer
{
    /// A 4x4 column-major floating-point matrix with right-handed cooordinates.
    class ALIMER_API Matrix4x4
    {
    public:
        union
        {
            float data[4][4];
            struct
            {
                float m11, m12, m13, m14;
                float m21, m22, m23, m24;
                float m31, m32, m33, m34;
                float m41, m42, m43, m44;
            };
        };

        /// Construct an identity matrix.
        Matrix4x4()  noexcept
            : m11(1.0f), m12(0.0f), m13(0.0f), m14(0.0f)
            , m21(0.0f), m22(1.0f), m23(0.0f), m24(0.0f)
            , m31(0.0f), m32(0.0f), m33(1.0f), m34(0.0f)
            , m41(0.0f), m42(0.0f), m43(0.0f), m44(1.0f)
        {

        }

        Matrix4x4(const Matrix4x4&) = default;
        Matrix4x4& operator=(const Matrix4x4&) = default;

        Matrix4x4(Matrix4x4&&) = default;
        Matrix4x4& operator=(Matrix4x4&&) = default;

        explicit inline Matrix4x4(float value)
            : m11(value), m12(0.0f), m13(0.0f), m14(0.0f)
            , m21(0.0f), m22(value), m23(0.0f), m24(0.0f)
            , m31(0.0f), m32(0.0f), m33(value), m34(0.0f)
            , m41(0.0f), m42(0.0f), m43(0.0f), m44(value)
        {
        }

        /// Construct from values.
        constexpr Matrix4x4(
            float m11_, float m12_, float m13_, float m14_,
            float m21_, float m22_, float m23_, float m24_,
            float m31_, float m32_, float m33_, float m34_,
            float m41_, float m42_, float m43_, float m44_)
            : m11(m11_), m12(m12_), m13(m13_), m14(m14_)
            , m21(m21_), m22(m22_), m23(m23_), m24(m24_)
            , m31(m31_), m32(m32_), m33(m33_), m34(m34_)
            , m41(m41_), m42(m42_), m43(m43_), m44(m44_)
        {
        }

        /// Construct from values.
        explicit Matrix4x4(const Vector3& row0, const Vector3& row1, const Vector3& row2)
            : m11(row0.x), m12(row0.y), m13(row0.z), m14(0.0f)
            , m21(row1.x), m22(row1.y), m23(row1.z), m24(0.0f)
            , m31(row2.x), m32(row2.y), m33(row2.z), m34(0.0f)
            , m41(0.0f), m42(0.0f), m43(0.0f), m44(1.0f)
        {
        }

        /// Construct from values.
        explicit Matrix4x4(
            const Vector4& row0,
            const Vector4& row1,
            const Vector4& row2,
            const Vector4& row3)
            : m11(row0.x), m12(row0.y), m13(row0.z), m14(row0.w)
            , m21(row1.x), m22(row1.y), m23(row1.z), m24(row1.w)
            , m31(row2.x), m32(row2.y), m33(row2.z), m34(row2.w)
            , m41(row3.x), m42(row3.y), m43(row3.z), m44(row3.w)
        {

        }

        /// Construct from float values.
        explicit Matrix4x4(_In_reads_(16) const float *data) noexcept
            : m11(data[0]), m12(data[1]), m13(data[2]), m14(data[3])
            , m21(data[4]), m22(data[5]), m23(data[6]), m24(data[7])
            , m31(data[8]), m32(data[9]), m33(data[10]), m34(data[11])
            , m41(data[12]), m42(data[13]), m43(data[14]), m44(data[15])
        {
        }

        float operator() (uint32_t row, uint32_t column) const { return data[row][column]; }
        float& operator() (uint32_t row, uint32_t column) { return data[row][column]; }

        // Comparison operators
        bool operator == (const Matrix4x4& rhs) const
        {
#ifdef ALIMER_SSE2
            __m128 c0 = _mm_cmpeq_ps(_mm_loadu_ps(&m11), _mm_loadu_ps(&rhs.m11));
            __m128 c1 = _mm_cmpeq_ps(_mm_loadu_ps(&m21), _mm_loadu_ps(&rhs.m21));
            c0 = _mm_and_ps(c0, c1);
            __m128 c2 = _mm_cmpeq_ps(_mm_loadu_ps(&m31), _mm_loadu_ps(&rhs.m31));
            __m128 c3 = _mm_cmpeq_ps(_mm_loadu_ps(&m41), _mm_loadu_ps(&rhs.m41));
            c2 = _mm_and_ps(c2, c3);
            c0 = _mm_and_ps(c0, c2);
            __m128 hi = _mm_movehl_ps(c0, c0);
            c0 = _mm_and_ps(c0, hi);
            hi = _mm_shuffle_ps(c0, c0, _MM_SHUFFLE(1, 1, 1, 1));
            c0 = _mm_and_ps(c0, hi);
            return _mm_cvtsi128_si32(_mm_castps_si128(c0)) == -1;
#else
            const float* leftData = Data();
            const float* rightData = rhs.Data();

            for (unsigned i = 0; i < 16; ++i)
            {
                if (leftData[i] != rightData[i])
                    return false;
            }

            return true;
#endif
        }

        bool operator != (const Matrix4x4& rhs) const { return !(*this == rhs); }

        /// Multiply a Vector3 which is assumed to represent position.
        Vector3 operator *(const Vector3& rhs) const
        {
#ifdef ALIMER_SSE2
            __m128 vec = _mm_set_ps(1.f, rhs.z, rhs.y, rhs.x);
            __m128 r0 = _mm_mul_ps(_mm_loadu_ps(&m11), vec);
            __m128 r1 = _mm_mul_ps(_mm_loadu_ps(&m21), vec);
            __m128 t0 = _mm_unpacklo_ps(r0, r1);
            __m128 t1 = _mm_unpackhi_ps(r0, r1);
            t0 = _mm_add_ps(t0, t1);
            __m128 r2 = _mm_mul_ps(_mm_loadu_ps(&m31), vec);
            __m128 r3 = _mm_mul_ps(_mm_loadu_ps(&m41), vec);
            __m128 t2 = _mm_unpacklo_ps(r2, r3);
            __m128 t3 = _mm_unpackhi_ps(r2, r3);
            t2 = _mm_add_ps(t2, t3);
            vec = _mm_add_ps(_mm_movelh_ps(t0, t2), _mm_movehl_ps(t2, t0));
            vec = _mm_div_ps(vec, _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(3, 3, 3, 3)));
            return Vector3(
                _mm_cvtss_f32(vec),
                _mm_cvtss_f32(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1))),
                _mm_cvtss_f32(_mm_movehl_ps(vec, vec)));
#else
            float invW = 1.0f / (m41 * rhs.x + m42 * rhs.y + m43 * rhs.z + m44);

            return Vector3(
                (m11 * rhs.x + m12 * rhs.y + m13 * rhs.z + m14) * invW,
                (m21 * rhs.x + m22 * rhs.y + m23 * rhs.z + m24) * invW,
                (m31 * rhs.x + m32 * rhs.y + m33 * rhs.z + m34) * invW
            );
#endif
        }

        /// Multiply a Vector4.
        Vector4 operator *(const Vector4& rhs) const
        {
#ifdef ALIMER_SSE2
            __m128 vec = _mm_loadu_ps(&rhs.x);
            __m128 r0 = _mm_mul_ps(_mm_loadu_ps(&m11), vec);
            __m128 r1 = _mm_mul_ps(_mm_loadu_ps(&m21), vec);
            __m128 t0 = _mm_unpacklo_ps(r0, r1);
            __m128 t1 = _mm_unpackhi_ps(r0, r1);
            t0 = _mm_add_ps(t0, t1);
            __m128 r2 = _mm_mul_ps(_mm_loadu_ps(&m31), vec);
            __m128 r3 = _mm_mul_ps(_mm_loadu_ps(&m41), vec);
            __m128 t2 = _mm_unpacklo_ps(r2, r3);
            __m128 t3 = _mm_unpackhi_ps(r2, r3);
            t2 = _mm_add_ps(t2, t3);
            vec = _mm_add_ps(_mm_movelh_ps(t0, t2), _mm_movehl_ps(t2, t0));

            Vector4 ret;
            _mm_storeu_ps(&ret.x, vec);
            return ret;
#else
            return Vector4(
                m11 * rhs.x + m21 * rhs.y + m31 * rhs.z + m41 * rhs.w,
                m21 * rhs.x + m22 * rhs.y + m23 * rhs.z + m24 * rhs.w,
                m31 * rhs.x + m32 * rhs.y + m33 * rhs.z + m34 * rhs.w,
                m41 * rhs.x + m42 * rhs.y + m43 * rhs.z + m44 * rhs.w
            );
#endif
        }

        /// Add a matrix.
        Matrix4x4 operator +(const Matrix4x4& rhs) const
        {
#ifdef ALIMER_SSE2
            Matrix4x4 result;
            _mm_storeu_ps(&result.m11, _mm_add_ps(_mm_loadu_ps(&m11), _mm_loadu_ps(&rhs.m11)));
            _mm_storeu_ps(&result.m21, _mm_add_ps(_mm_loadu_ps(&m21), _mm_loadu_ps(&rhs.m21)));
            _mm_storeu_ps(&result.m31, _mm_add_ps(_mm_loadu_ps(&m31), _mm_loadu_ps(&rhs.m31)));
            _mm_storeu_ps(&result.m41, _mm_add_ps(_mm_loadu_ps(&m41), _mm_loadu_ps(&rhs.m41)));
            return result;
#else
            return Matrix4x4(
                m11 + rhs.m12,
                m12 + rhs.m12,
                m13 + rhs.m13,
                m14 + rhs.m14,
                m21 + rhs.m21,
                m22 + rhs.m22,
                m23 + rhs.m23,
                m24 + rhs.m24,
                m31 + rhs.m31,
                m32 + rhs.m32,
                m33 + rhs.m33,
                m34 + rhs.m34,
                m41 + rhs.m41,
                m42 + rhs.m42,
                m43 + rhs.m43,
                m44 + rhs.m44
            );
#endif
        }

        /// Subtract a matrix.
        Matrix4x4 operator -(const Matrix4x4& rhs) const
        {
#ifdef ALIMER_SSE2
            Matrix4x4 result;
            _mm_storeu_ps(&result.m11, _mm_sub_ps(_mm_loadu_ps(&m11), _mm_loadu_ps(&rhs.m11)));
            _mm_storeu_ps(&result.m21, _mm_sub_ps(_mm_loadu_ps(&m21), _mm_loadu_ps(&rhs.m21)));
            _mm_storeu_ps(&result.m31, _mm_sub_ps(_mm_loadu_ps(&m31), _mm_loadu_ps(&rhs.m31)));
            _mm_storeu_ps(&result.m41, _mm_sub_ps(_mm_loadu_ps(&m41), _mm_loadu_ps(&rhs.m41)));
            return result;
#else
            return Matrix4x4(
                m11 - rhs.m12,
                m12 - rhs.m12,
                m13 - rhs.m13,
                m14 - rhs.m14,
                m21 - rhs.m21,
                m22 - rhs.m22,
                m23 - rhs.m23,
                m24 - rhs.m24,
                m31 - rhs.m31,
                m32 - rhs.m32,
                m33 - rhs.m33,
                m34 - rhs.m34,
                m41 - rhs.m41,
                m42 - rhs.m42,
                m43 - rhs.m43,
                m44 - rhs.m44
            );
#endif
        }

        /// Multiply with a scalar.
        Matrix4x4 operator *(float rhs) const
        {
#ifdef ALIMER_SSE2
            const __m128 mul = _mm_set1_ps(rhs);
            Matrix4x4 result;
            _mm_storeu_ps(&result.m11, _mm_mul_ps(_mm_loadu_ps(&m11), mul));
            _mm_storeu_ps(&result.m21, _mm_mul_ps(_mm_loadu_ps(&m21), mul));
            _mm_storeu_ps(&result.m31, _mm_mul_ps(_mm_loadu_ps(&m31), mul));
            _mm_storeu_ps(&result.m41, _mm_mul_ps(_mm_loadu_ps(&m41), mul));
            return result;
#else
            return Matrix4x4(
                m11 * rhs,
                m12 * rhs,
                m13 * rhs,
                m14 * rhs,
                m21 * rhs,
                m22 * rhs,
                m23 * rhs,
                m24 * rhs,
                m31 * rhs,
                m32 * rhs,
                m33 * rhs,
                m34 * rhs,
                m41 * rhs,
                m42 * rhs,
                m43 * rhs,
                m44 * rhs
            );
#endif
        }

        /// Multiply a matrix.
        Matrix4x4 operator *(const Matrix4x4& rhs) const
        {
#ifdef ALIMER_SSE2
            Matrix4x4 result;

            __m128 r0 = _mm_loadu_ps(&rhs.m11);
            __m128 r1 = _mm_loadu_ps(&rhs.m21);
            __m128 r2 = _mm_loadu_ps(&rhs.m31);
            __m128 r3 = _mm_loadu_ps(&rhs.m41);

            __m128 l = _mm_loadu_ps(&m11);
            __m128 t0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);
            __m128 t1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);
            __m128 t2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);
            __m128 t3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);
            _mm_storeu_ps(&result.m11, _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3)));

            l = _mm_loadu_ps(&m21);
            t0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);
            t1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);
            t2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);
            t3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);
            _mm_storeu_ps(&result.m21, _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3)));

            l = _mm_loadu_ps(&m31);
            t0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);
            t1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);
            t2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);
            t3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);
            _mm_storeu_ps(&result.m31, _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3)));

            l = _mm_loadu_ps(&m41);
            t0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);
            t1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);
            t2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);
            t3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);
            _mm_storeu_ps(&result.m41, _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3)));
            return result;
#else
            return Matrix4x4(
                m11 * rhs.m11 + m12 * rhs.m21 + m13 * rhs.m31 + m14 * rhs.m41,
                m11 * rhs.m12 + m12 * rhs.m22 + m13 * rhs.m32 + m14 * rhs.m42,
                m11 * rhs.m13 + m12 * rhs.m23 + m13 * rhs.m33 + m14 * rhs.m43,
                m11 * rhs.m14 + m12 * rhs.m24 + m13 * rhs.m34 + m14 * rhs.m44,
                m21 * rhs.m11 + m22 * rhs.m21 + m23 * rhs.m31 + m24 * rhs.m41,
                m21 * rhs.m12 + m22 * rhs.m22 + m23 * rhs.m32 + m24 * rhs.m42,
                m21 * rhs.m13 + m22 * rhs.m23 + m23 * rhs.m33 + m24 * rhs.m43,
                m21 * rhs.m14 + m22 * rhs.m24 + m23 * rhs.m34 + m24 * rhs.m44,
                m31 * rhs.m11 + m32 * rhs.m21 + m33 * rhs.m31 + m34 * rhs.m41,
                m31 * rhs.m12 + m32 * rhs.m22 + m33 * rhs.m32 + m34 * rhs.m42,
                m31 * rhs.m13 + m32 * rhs.m23 + m33 * rhs.m33 + m34 * rhs.m43,
                m31 * rhs.m14 + m32 * rhs.m24 + m33 * rhs.m34 + m34 * rhs.m44,
                m41 * rhs.m11 + m42 * rhs.m21 + m43 * rhs.m31 + m44 * rhs.m41,
                m41 * rhs.m12 + m42 * rhs.m22 + m43 * rhs.m32 + m44 * rhs.m42,
                m41 * rhs.m13 + m42 * rhs.m23 + m43 * rhs.m33 + m44 * rhs.m43,
                m41 * rhs.m14 + m42 * rhs.m24 + m43 * rhs.m34 + m44 * rhs.m44
            );
#endif
        }

        /**
        * Sets this matrix to the identity matrix.
        */
        void SetIdentity();

        /**
        * Sets all elements of the current matrix to zero.
        */
        void SetZero();


        /// Return float data.
        const float* Data() const { return &m11; }

        /// Return matrix element.
        float Element(uint32_t row, uint32_t column) const { return data[row][column]; }

        /// Return matrix row.
        Vector4 Row(uint32_t row) const { return Vector4(data[row][0], data[row][1], data[row][2], data[row][3]); }

        /// Return matrix column.
        Vector4 Column(uint32_t column) const { return Vector4(data[0][column], data[1][column], data[2][column], data[3][column]); }

        // Properties
        Vector3 Up() const { return Vector3(m21, m22, m23); }
        void SetUp(const Vector3& v) { m21 = v.x; m22 = v.y; m23 = v.z; }

        Vector3 Down() const { return Vector3(-m21, -m22, -m23); }
        void SetDown(const Vector3& v) { m21 = -v.x; m22 = -v.y; m23 = -v.z; }

        Vector3 Right() const { return Vector3(m11, m12, m13); }
        void SetRight(const Vector3& v) { m11 = v.x; m12 = v.y; m13 = v.z; }

        Vector3 Left() const { return Vector3(-m11, -m12, -m13); }
        void SetLeft(const Vector3& v) { m11 = -v.x; m12 = -v.y; m13 = -v.z; }

        Vector3 Forward() const { return Vector3(-m31, -m32, -m33); }
        void SetForward(const Vector3& v) { m31 = -v.x; m32 = -v.y; m33 = -v.z; }

        Vector3 Backward() const { return Vector3(m31, m32, m33); }
        void SetBackward(const Vector3& v) { m31 = v.x; m32 = v.y; m33 = v.z; }

        Vector3 Translation() const { return Vector3(m14, m24, m34); }
        void SetTranslation(const Vector3& v) { m14 = v.x; m24 = v.y; m34 = v.z; }

        /// Return the scaling part.
        Vector3 Scale() const
        {
            return Vector3(
                sqrtf(m11 * m11 + m21 * m21 + m31 * m31),
                sqrtf(m12 * m12 + m22 * m22 + m32 * m32),
                sqrtf(m13 * m13 + m23 * m23 + m33 * m33)
            );
        }

        /// Set scaling elements.
        void SetScale(const Vector3& scale)
        {
            m11 = scale.x;
            m22 = scale.y;
            m33 = scale.z;
        }

        /// Set uniform scaling elements.
        void SetScale(float scale)
        {
            m11 = scale;
            m22 = scale;
            m33 = scale;
        }

        /// Return as string.
        String ToString() const;

        Matrix4x4 Transpose() const;
        void Transpose(Matrix4x4& result) const;

        Matrix4x4 Inverse() const;

        static Matrix4x4 CreateLookAt(const vec3 &eye, const vec3 &target, const vec3 &up);

        static Matrix4x4 CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance);

        static Matrix4x4 CreateRotationX(float radians);
        static Matrix4x4 CreateRotationY(float radians);
        static Matrix4x4 CreateRotationZ(float radians);

        // Constants
        static const Matrix4x4 Identity;
        static const Matrix4x4 Zero;
    };

    /// Multiply a 4x4 matrix with a scalar.
    inline Matrix4x4 operator *(float lhs, const Matrix4x4& rhs) { return rhs * lhs; }

    // GLSL/HLSL style
    using mat4 = Matrix4x4;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
