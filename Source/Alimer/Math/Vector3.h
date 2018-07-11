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

#include "../AlimerConfig.h"
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

    using vec3 = Vector3;
}

