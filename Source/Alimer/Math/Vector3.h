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

#include "../Math/MathUtil.h"
#include <string>

namespace Alimer
{
    /// Defines a 3D vector
    class ALIMER_API Vector3
    {
    public:
        /// The x coordinate.
        float x;

        /// The y coordinate.
        float y;

        /// The z coordinate.
        float z;

        Vector3() noexcept : x(0.0f), y(0.0f), z(0.0f) {}

        Vector3(const Vector3&) = default;
        Vector3& operator=(const Vector3&) = default;

        Vector3(Vector3&&) = default;
        Vector3& operator=(Vector3&&) = default;

        constexpr explicit Vector3(float value) : x(value), y(value), z(value) {}
        constexpr Vector3(float x_, float y_) : x(x_), y(y_), z(0.0f) {}
        constexpr Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
        explicit Vector3(const float *data) : x(data[0]), y(data[1]), z(data[2]) {}

        // Comparison operators
        bool operator == (const Vector3& rhs)  const { return x == rhs.x && y == rhs.y && z == rhs.z; }
        bool operator != (const Vector3& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }

        /// Add a vector.
        Vector3 operator +(const Vector3& rhs) const { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }

        /// Return negation.
        Vector3 operator -() const { return Vector3(-x, -y, -z); }

        /// Subtract a vector.
        Vector3 operator -(const Vector3& rhs) const { return Vector3(x - rhs.x, y - rhs.y, z - rhs.z); }

        /// Multiply with a scalar.
        Vector3 operator *(float rhs) const { return Vector3(x * rhs, y * rhs, z * rhs); }

        /// Multiply with a vector.
        Vector3 operator *(const Vector3& rhs) const { return Vector3(x * rhs.x, y * rhs.y, z * rhs.z); }

        /// Divide by a scalar.
        Vector3 operator /(float rhs) const { return Vector3(x / rhs, y / rhs, z / rhs); }

        /// Divide by a vector.
        Vector3 operator /(const Vector3& rhs) const { return Vector3(x / rhs.x, y / rhs.y, z / rhs.z); }

        /// Add-assign a vector.
        Vector3& operator +=(const Vector3& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }

        /// Subtract-assign a vector.
        Vector3& operator -=(const Vector3& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        /// Multiply-assign a scalar.
        Vector3& operator *=(float scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        /// Multiply-assign a vector.
        Vector3& operator *=(const Vector3& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            return *this;
        }

        /// Divide-assign a scalar.
        Vector3& operator /=(float scalar)
        {
            ALIMER_ASSERT(scalar != 0.0f);
            float invRhs = 1.0f / scalar;
            x *= invRhs;
            y *= invRhs;
            z *= invRhs;
            return *this;
        }

        /// Divide-assign a vector.
        Vector3& operator /=(const Vector3& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            return *this;
        }

        /// Gets the length computed for this vector.
        float Length() const { return sqrtf(x * x + y * y + z * z); }

        /// Gets the squared length computed for this vector.
        float LengthSquared() const { return x * x + y * y + z * z; }

        /// Return float data.
        const float* Data() const { return &x; }

        static void Add(const Vector3& left, const Vector3& right, Vector3* result);
        static Vector3 Add(const Vector3& left, const Vector3& right);

        static void Subtract(const Vector3& left, const Vector3& right, Vector3* result);
        static Vector3 Subtract(const Vector3& left, const Vector3& right);

        static void Multiply(const Vector3& left, const Vector3& right, Vector3* result);
        static Vector3 Multiply(const Vector3& left, const Vector3& right);

        static void Divide(const Vector3& left, const Vector3& right, Vector3* result);
        static Vector3 Divide(const Vector3& left, const Vector3& right);

        /// Normalizes the given vector.
        void Normalize()
        {
            float lenSquared = LengthSquared();
            if (!Alimer::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                x *= invLen;
                y *= invLen;
                z *= invLen;
            }
        }

        /// Return normalized to unit length.
        Vector3 Normalized() const
        {
            float lenSquared = LengthSquared();
            if (!Alimer::Equals(lenSquared, 1.0f)
                && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                return *this * invLen;
            }

            return *this;
        }


        static void Cross(const Vector3& left, const Vector3& right, Vector3* result);
        static Vector3 Cross(const Vector3& left, const Vector3& right);

        /// Calculate dot product.
        float Dot(const Vector3& rhs) const { return (x * rhs.x) + (y * rhs.y) + (z * rhs.z); }

        /// Calculate dot product between two vectors.
        static float Dot(const Vector3& left, const Vector3& right);

        // Constants
        static const Vector3 Zero;
        static const Vector3 One;
        static const Vector3 UnitX;
        static const Vector3 UnitY;
        static const Vector3 UnitZ;
        static const Vector3 Up;
        static const Vector3 Down;
        static const Vector3 Right;
        static const Vector3 Left;
        static const Vector3 Forward;
        static const Vector3 Backward;
    };

    /// Multiply Vector3 with a scalar.
    inline Vector3 operator *(float lhs, const Vector3& rhs) { return rhs * lhs; }

    using float3 = Vector3;
    using vec3 = Vector3;
}

