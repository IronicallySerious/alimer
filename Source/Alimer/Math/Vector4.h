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
    /// Defines a 4D vector
    class ALIMER_API Vector4
    {
    public:
        /// The x coordinate.
        float x;

        /// The y coordinate.
        float y;

        /// The z coordinate.
        float z;

        /// The w coordinate.
        float w;

        Vector4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

        Vector4(const Vector4&) = default;
        Vector4& operator=(const Vector4&) = default;

        Vector4(Vector4&&) = default;
        Vector4& operator=(Vector4&&) = default;

        constexpr explicit Vector4(float value) : x(value), y(value), z(value), w(value) {}
        constexpr Vector4(float x_, float y_) : x(x_), y(y_), z(0.0f), w(0.0f) {}
        constexpr Vector4(float x_, float y_, float z_) : x(x_), y(y_), z(z_), w(0.0f) {}
        constexpr Vector4(float x_, float y_, float z_ , float w_) : x(x_), y(y_), z(z_), w(w_) {}
        explicit Vector4(const float *data) : x(data[0]), y(data[1]), z(data[2]), w(data[3]) {}

        // Comparison operators
        inline bool operator == (const Vector4& rhs)  const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
        inline bool operator != (const Vector4& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w; }


        // Constants
        static const Vector4 Zero;
        static const Vector4 One;
        static const Vector4 UnitX;
        static const Vector4 UnitY;
        static const Vector4 UnitZ;
        static const Vector4 UnitW;
    };

    using vec4 = Vector4;
}

