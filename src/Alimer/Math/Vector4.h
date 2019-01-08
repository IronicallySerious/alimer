//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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

namespace alimer
{
    /// Defines a 4 elements floating-point vector.
    class ALIMER_API Vector4
    {
    public:
        /// The x-coordinate of the vector.
        float x;
        /// The y-coordinate of the vector.
        float y;
        /// The z-coordinate of the vector.
        float z;
        /// The w-coordinate of the vector.
        float w;

        /// Construct a zero vector.
        Vector4()  noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        constexpr explicit Vector4(float value) : x(value), y(value), z(value), w(value) {}

        /// Construct from coordinates.
        constexpr Vector4(float x_, float y_, float z_, float w_) noexcept
            : x(x_), y(y_), z(z_), w(w_) 
        {
        }

        /// Construct from a float array.
        explicit Vector4(const float *data) 
            : x(data[0]), y(data[1]), z(data[2]), w(data[3])
        {
        }

        Vector4(const Vector4&) = default;
        Vector4& operator=(const Vector4&) = default;
        Vector4(Vector4&&) = default;
        Vector4& operator=(Vector4&&) = default;

        // Comparison operators
        bool operator == (const Vector4& rhs) const { return (x == rhs.x) && (y == rhs.y) && (z== rhs.z) && (w == rhs.w); }
        bool operator != (const Vector4& rhs) const { return (x != rhs.x) || (y != rhs.y) || (z != rhs.z) || (w != rhs.w); }

        /// Add a vector.
        Vector4 operator +(const Vector4& rhs) const { return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }

        /// Return negation.
        Vector4 operator -() const { return Vector4(-x, -y, -z, -w); }

        /// Subtract a vector.
        Vector4 operator -(const Vector4& rhs) const { return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }

        /// Multiply with a scalar.
        Vector4 operator *(float rhs) const { return Vector4(x * rhs, y * rhs, z * rhs, w * rhs); }

        /// Multiply with a vector.
        Vector4 operator *(const Vector4& rhs) const { return Vector4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }

        /// Divide by a scalar.
        Vector4 operator /(float rhs) const { return Vector4(x / rhs, y / rhs, z / rhs, w / rhs); }

        /// Divide by a vector.
        Vector4 operator /(const Vector4& rhs) const { return Vector4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

        /// Add-assign a vector.
        Vector4& operator +=(const Vector4& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            w += rhs.w;
            return *this;
        }

        /// Subtract-assign a vector.
        Vector4& operator -=(const Vector4& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            w -= rhs.w;
            return *this;
        }

        /// Multiply-assign a scalar.
        Vector4& operator *=(float rhs)
        {
            x *= rhs;
            y *= rhs;
            z *= rhs;
            w *= rhs;
            return *this;
        }

        /// Multiply-assign a vector.
        Vector4& operator *=(const Vector4& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            w *= rhs.w;
            return *this;
        }

        /// Divide-assign a scalar.
        Vector4& operator /=(float rhs)
        {
            float invRhs = 1.0f / rhs;
            x *= invRhs;
            y *= invRhs;
            z *= invRhs;
            w *= invRhs;
            return *this;
        }

        /// Divide-assign a vector.
        Vector4& operator /=(const Vector4& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            w /= rhs.w;
            return *this;
        }

        /// Normalize to unit length.
        void Normalize()
        {
            float lenSquared = LengthSquared();
            if (!alimer::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invNorm = 1.0f / sqrtf(lenSquared);
                x *= invNorm;
                y *= invNorm;
            }
        }

        /// Return length.
        float Length() const { return sqrtf(x * x + y * y + z * z + w * w); }

        /// Return squared length.
        float LengthSquared() const { return x * x + y * y + z * z + w * w; }

        /// Test for equality with another vector with epsilon.
        bool Equals(const Vector4& rhs) const {
            return alimer::Equals(x, rhs.x) 
                && alimer::Equals(y, rhs.y)
                && alimer::Equals(z, rhs.z)
                && alimer::Equals(w, rhs.w);
        }

        /// Linear interpolation with another vector.
        Vector4 Lerp(const Vector4& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }

        /// Return whether is zero.
        bool IsZero() const { 
            return alimer::Equals(x, 0.0f)
                && alimer::Equals(y, 0.0f)
                && alimer::Equals(z, 0.0f)
                && alimer::Equals(w, 0.0f);
        }

        /// Return whether is NaN.
        bool IsNaN() const { 
            return alimer::IsNaN(x) 
                || alimer::IsNaN(y)
                || alimer::IsNaN(z)
                || alimer::IsNaN(w);
        }

        /// Return normalized to unit length.
        Vector4 Normalized() const
        {
            float lenSquared = LengthSquared();
            if (!alimer::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                return *this * invLen;
            }
            
            return *this;
        }
        
        /// Return as string.
        std::string ToString() const;

        /// Return float data.
        const float* Data() const { return &x; }

        // Constants
        static const Vector4 Zero;
        static const Vector4 One;
        static const Vector4 UnitX;
        static const Vector4 UnitY;
        static const Vector4 UnitZ;
        static const Vector4 UnitW;
    };

    /// Multiply Vector4 with a scalar
    inline Vector4 operator *(float lhs, const Vector4& rhs) { return rhs * lhs; }
}
