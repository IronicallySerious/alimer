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

#include "../Math/Vector2.h"
#include "../Math/MathUtil.h"

namespace alimer
{
    /// Defines a 3 elements integer vector.
    class ALIMER_API IntVector3
    {
    public:
        /// The x-coordinate of the vector.
        int x;
        /// The y-coordinate of the vector.
        int y;
        /// The z-coordinate of the vector.
        int z;

        /// Construct an identity matrix.
        IntVector3()  noexcept : x(0), y(0), z(0) {}
        constexpr explicit IntVector3(int value) : x(value), y(value),z(value) {}
        constexpr IntVector3(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
        explicit IntVector3(const int *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]) {}

        IntVector3(const IntVector3&) = default;
        IntVector3& operator=(const IntVector3&) = default;
        IntVector3(IntVector3&&) = default;
        IntVector3& operator=(IntVector3&&) = default;

        // Comparison operators
        bool operator == (const IntVector3& rhs) const { return (x == rhs.x) && (y == rhs.y) && (z == rhs.z); }
        bool operator != (const IntVector3& rhs) const { return (x != rhs.x) || (y != rhs.y) || (z != rhs.z); }

        /// Add a vector.
        IntVector3 operator +(const IntVector3& rhs) const { return IntVector3(x + rhs.x, y + rhs.y, z + rhs.z); }

        /// Return negation.
        IntVector3 operator -() const { return IntVector3(-x, -y, -z); }

        /// Subtract a vector.
        IntVector3 operator -(const IntVector3& rhs) const { return IntVector3(x - rhs.x, y - rhs.y, z - rhs.z); }

        /// Multiply with a scalar.
        IntVector3 operator *(int rhs) const { return IntVector3(x * rhs, y * rhs, z * rhs); }

        /// Multiply with a vector.
        IntVector3 operator *(const IntVector3& rhs) const { return IntVector3(x * rhs.x, y * rhs.y, z * rhs.z); }

        /// Divide by a scalar.
        IntVector3 operator /(int rhs) const { return IntVector3(x / rhs, y / rhs, z / rhs); }

        /// Divide by a vector.
        IntVector3 operator /(const IntVector3& rhs) const { return IntVector3(x / rhs.x, y / rhs.y, z / rhs.z); }

        /// Add-assign a vector.
        IntVector3& operator +=(const IntVector3& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }

        /// Subtract-assign a vector.
        IntVector3& operator -=(const IntVector3& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        /// Multiply-assign a scalar.
        IntVector3& operator *=(int rhs)
        {
            x *= rhs;
            y *= rhs;
            z *= rhs;
            return *this;
        }

        /// Multiply-assign a vector.
        IntVector3& operator *=(const IntVector3& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            return *this;
        }

        /// Divide-assign a scalar.
        IntVector3& operator /=(int rhs)
        {
            x /= rhs;
            y /= rhs;
            z /= rhs;
            return *this;
        }

        /// Divide-assign a vector.
        IntVector3& operator /=(const IntVector3& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            return *this;
        }

        bool IsZero() const { return (x == 0) && (y == 0) && (z == 0); }

        /// Return as string.
        std::string ToString() const;

        /// Return integer data.
        const int* Data() const { return &x; }

        // Constants
        static const IntVector3 Zero;
        static const IntVector3 One;
        static const IntVector3 UnitX;
        static const IntVector3 UnitY;
        static const IntVector3 UnitZ;
        static const IntVector3 Up;
        static const IntVector3 Down;
        static const IntVector3 Right;
        static const IntVector3 Left;
        static const IntVector3 Forward;
        static const IntVector3 Backward;
    };

    /// Defines a 3 elements floating-point vector.
    class ALIMER_API Vector3
    {
    public:
        /// The x-coordinate of the vector.
        float x;
        /// The y-coordinate of the vector.
        float y;
        /// The z-coordinate of the vector.
        float z;

        /// Construct a zero vector.
        Vector3()  noexcept : x(0.0f), y(0.0f),z(0.0f) 
        {
        }

        constexpr explicit Vector3(float value) noexcept 
            : x(value), y(value),z(value)
        {
        }

        /// Construct from coordinates.
        constexpr Vector3(float x_, float y_, float z_) noexcept 
            : x(x_), y(y_),z(z_) 
        {
        }

        /// Construct from a two-dimensional vector and the Z coordinate.
        constexpr Vector3(const Vector2& vector, float z) noexcept
            : x(vector.x), y(vector.y), z(z)
        {
        }

        /// Construct from a two-dimensional vector.
        explicit Vector3(const Vector2& vector) noexcept 
            : x(vector.x), y(vector.y), z(0.0f)
        {
        }

        /// Construct from an IntVector3.
        explicit Vector3(const IntVector3& vector) noexcept
            : x((float)vector.x), y((float)vector.y), z((float)vector.z)
        {
        }

        /// Construct from two-dimensional coordinates.
        Vector3(float x_, float y_) noexcept
            : x(x_), y(y_), z(0.0f)
        {
        }

        /// Construct from a float array.
        explicit Vector3(const float *data) 
            : x(data[0]), y(data[1]), z(data[2])
        {
        }

        Vector3(const Vector3&) = default;
        Vector3& operator=(const Vector3&) = default;
        Vector3(Vector3&&) = default;
        Vector3& operator=(Vector3&&) = default;

        // Comparison operators
        bool operator == (const Vector3& rhs) const { return (x == rhs.x) && (y == rhs.y) && (z == rhs.z); }
        bool operator != (const Vector3& rhs) const { return (x != rhs.x) || (y != rhs.y) || (z != rhs.z); }

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
        Vector3& operator *=(float rhs)
        {
            x *= rhs;
            y *= rhs;
            z *= rhs;
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
        Vector3& operator /=(float rhs)
        {
            float invRhs = 1.0f / rhs;
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

        /// Normalize to unit length.
        void Normalize()
        {
            float lenSquared = LengthSquared();
            if (!alimer::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                x *= invLen;
                y *= invLen;
                z *= invLen;
            }
        }

        /// Return length.
        float Length() const { return sqrtf(x * x + y * y + z * z); }

        /// Return squared length.
        float LengthSquared() const { return x * x + y * y + z * z; }

        /// Test for equality with another vector with epsilon.
        bool Equals(const Vector3& rhs) const { 
            return alimer::Equals(x, rhs.x) 
                && alimer::Equals(y, rhs.y) 
                && alimer::Equals(z, rhs.z);
        }

        /// Linear interpolation with another vector.
        Vector3 Lerp(const Vector3& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }

        /// Return whether is zero.
        bool IsZero() const { return alimer::Equals(x, 0.0f) && alimer::Equals(y, 0.0f) && alimer::Equals(z, 0.0f); }

        /// Return whether is NaN.
        bool IsNaN() const { return alimer::IsNaN(x) || alimer::IsNaN(y) || alimer::IsNaN(z); }

        /// Return normalized to unit length.
        Vector3 Normalized() const
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

    /// Multiply Vector3 with a scalar
    inline Vector3 operator *(float lhs, const Vector3& rhs) { return rhs * lhs; }

    /// Multiply IntVector3 with a scalar.
    inline IntVector3 operator *(int lhs, const IntVector3& rhs) { return rhs * lhs; }

}
