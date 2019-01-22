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

#include "../Math/MathUtil.h"
#include <string>

namespace alimer
{
    /// Defines a 2 elements integer vector.
    class ALIMER_API IntVector2
    {
    public:
        /// The x-coordinate of the vector.
        int x;
        /// The y-coordinate of the vector.
        int y;

        /// Construct a zero vector.
        IntVector2()  noexcept : x(0), y(0) {}
        constexpr explicit IntVector2(int value) : x(value), y(value) {}
        constexpr IntVector2(int x_, int y_) : x(x_), y(y_) {}
        explicit IntVector2(const int *pArray) : x(pArray[0]), y(pArray[1]) {}

        IntVector2(const IntVector2&) = default;
        IntVector2& operator=(const IntVector2&) = default;
        IntVector2(IntVector2&&) = default;
        IntVector2& operator=(IntVector2&&) = default;

        // Comparison operators
        bool operator == (const IntVector2& rhs) const { return (x == rhs.x) && (y == rhs.y); }
        bool operator != (const IntVector2& rhs) const { return (x != rhs.x) || (y != rhs.y); }

        /// Add a vector.
        IntVector2 operator +(const IntVector2& rhs) const { return IntVector2(x + rhs.x, y + rhs.y); }

        /// Return negation.
        IntVector2 operator -() const { return IntVector2(-x, -y); }

        /// Subtract a vector.
        IntVector2 operator -(const IntVector2& rhs) const { return IntVector2(x - rhs.x, y - rhs.y); }

        /// Multiply with a scalar.
        IntVector2 operator *(int rhs) const { return IntVector2(x * rhs, y * rhs); }

        /// Multiply with a vector.
        IntVector2 operator *(const IntVector2& rhs) const { return IntVector2(x * rhs.x, y * rhs.y); }

        /// Divide by a scalar.
        IntVector2 operator /(int rhs) const { return IntVector2(x / rhs, y / rhs); }

        /// Divide by a vector.
        IntVector2 operator /(const IntVector2& rhs) const { return IntVector2(x / rhs.x, y / rhs.y); }

        /// Add-assign a vector.
        IntVector2& operator +=(const IntVector2& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        /// Subtract-assign a vector.
        IntVector2& operator -=(const IntVector2& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        /// Multiply-assign a scalar.
        IntVector2& operator *=(int rhs)
        {
            x *= rhs;
            y *= rhs;
            return *this;
        }

        /// Multiply-assign a vector.
        IntVector2& operator *=(const IntVector2& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            return *this;
        }

        /// Divide-assign a scalar.
        IntVector2& operator /=(int rhs)
        {
            x /= rhs;
            y /= rhs;
            return *this;
        }

        /// Divide-assign a vector.
        IntVector2& operator /=(const IntVector2& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            return *this;
        }

        bool IsZero() const { return (x == 0) && (y == 0); }

        /// Return as string.
        std::string ToString() const;

        /// Return integer data.
        const int* Data() const { return &x; }

        // Constants
        static const IntVector2 Zero;
        static const IntVector2 One;
        static const IntVector2 UnitX;
        static const IntVector2 UnitY;
    };

    /// Defines a 2 elements floating-point vector.
    class ALIMER_API Vector2
    {
    public:
        /// The x-coordinate of the vector.
        float x;
        /// The y-coordinate of the vector.
        float y;

        /// Construct a zero vector.
        Vector2()  noexcept : x(0.0f), y(0.0f) {}
        constexpr explicit Vector2(float value) : x(value), y(value) {}
        constexpr Vector2(float x_, float y_) : x(x_), y(y_) {}
        explicit Vector2(const float *pArray) : x(pArray[0]), y(pArray[1]) {}

        Vector2(const Vector2&) = default;
        Vector2& operator=(const Vector2&) = default;
        Vector2(Vector2&&) = default;
        Vector2& operator=(Vector2&&) = default;

        // Comparison operators
        bool operator == (const Vector2& rhs) const { return (x == rhs.x) && (y == rhs.y); }
        bool operator != (const Vector2& rhs) const { return (x != rhs.x) || (y != rhs.y); }

        /// Add a vector.
        Vector2 operator +(const Vector2& rhs) const { return Vector2(x + rhs.x, y + rhs.y); }

        /// Return negation.
        Vector2 operator -() const { return Vector2(-x, -y); }

        /// Subtract a vector.
        Vector2 operator -(const Vector2& rhs) const { return Vector2(x - rhs.x, y - rhs.y); }

        /// Multiply with a scalar.
        Vector2 operator *(float rhs) const { return Vector2(x * rhs, y * rhs); }

        /// Multiply with a vector.
        Vector2 operator *(const Vector2& rhs) const { return Vector2(x * rhs.x, y * rhs.y); }

        /// Divide by a scalar.
        Vector2 operator /(float rhs) const { return Vector2(x / rhs, y / rhs); }

        /// Divide by a vector.
        Vector2 operator /(const Vector2& rhs) const { return Vector2(x / rhs.x, y / rhs.y); }

        /// Add-assign a vector.
        Vector2& operator +=(const Vector2& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        /// Subtract-assign a vector.
        Vector2& operator -=(const Vector2& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        /// Multiply-assign a scalar.
        Vector2& operator *=(float rhs)
        {
            x *= rhs;
            y *= rhs;
            return *this;
        }

        /// Multiply-assign a vector.
        Vector2& operator *=(const Vector2& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            return *this;
        }

        /// Divide-assign a scalar.
        Vector2& operator /=(float rhs)
        {
            float invRhs = 1.0f / rhs;
            x *= invRhs;
            y *= invRhs;
            return *this;
        }

        /// Divide-assign a vector.
        Vector2& operator /=(const Vector2& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
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
        float Length() const { return sqrtf(x * x + y * y); }

        /// Return squared length.
        float LengthSquared() const { return x * x + y * y; }

        /// Test for equality with another vector with epsilon.
        bool Equals(const Vector2& rhs) const { return alimer::Equals(x, rhs.x) && alimer::Equals(y, rhs.y); }

        /// Linear interpolation with another vector.
        Vector2 Lerp(const Vector2& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }

        /// Return whether is zero.
        bool IsZero() const { return alimer::Equals(x, 0.0f) && alimer::Equals(y, 0.0f); }

        /// Return whether is NaN.
        bool IsNaN() const { return alimer::IsNaN(x) || alimer::IsNaN(y); }

        /// Return normalized to unit length.
        Vector2 Normalized() const
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
        static const Vector2 Zero;
        static const Vector2 One;
        static const Vector2 UnitX;
        static const Vector2 UnitY;
    };

    /// Multiply Vector2 with a scalar
    inline Vector2 operator *(float lhs, const Vector2& rhs) { return rhs * lhs; }

    /// Multiply IntVector2 with a scalar.
    inline IntVector2 operator *(int lhs, const IntVector2& rhs) { return rhs * lhs; }

}
