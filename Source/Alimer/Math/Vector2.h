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
    /// Defines a 2D vector.
    class ALIMER_API Vector2
    {
    public:
        /// The x coordinate.
        float x;

        /// The y coordinate.
        float y;

        Vector2() noexcept : x(0.0f), y(0.0f) {}

        Vector2(const Vector2&) = default;
        Vector2& operator=(const Vector2&) = default;

        Vector2(Vector2&&) = default;
        Vector2& operator=(Vector2&&) = default;

        constexpr explicit Vector2(float value) : x(value), y(value) {}
        constexpr Vector2(float x_, float y_) : x(x_), y(y_) {}
        explicit Vector2(const float *data) : x(data[0]), y(data[1]) {}

        // Comparison operators
        bool operator == (const Vector2& rhs)  const { return x == rhs.x && y == rhs.y; }
        bool operator != (const Vector2& rhs) const { return x != rhs.x || y != rhs.y; }

        // Constants
        static const Vector2 Zero;
        static const Vector2 One;
        static const Vector2 UnitX;
        static const Vector2 UnitY;
    };

    using vec2 = Vector2;
}

