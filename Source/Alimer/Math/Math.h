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

#include <functional>
#include <assert.h>
#include <memory.h>
#include "../Math/Vector2.h"
#include <string>

namespace Alimer
{
    struct Matrix4;
    struct Quaternion;
    struct Plane;

    struct Rectangle
    {
        int x;
        int y;
        int width;
        int height;

        Rectangle() noexcept : x(0), y(0), width(0), height(0) {}

        constexpr Rectangle(int width_, int height_) : x(0), y(0), width(width_), height(height_) {}
        constexpr Rectangle(int x_, int y_, int width_, int height_) : x(x_), y(y_), width(width_), height(height_) {}

        Rectangle(const Rectangle&) = default;
        Rectangle& operator=(const Rectangle&) = default;

        Rectangle(Rectangle&&) = default;
        Rectangle& operator=(Rectangle&&) = default;

        // Comparison operators
        bool operator == (const Rectangle& r) const { return (x == r.x) && (y == r.y) && (width == r.width) && (height == r.height); }
        bool operator != (const Rectangle& r) const { return (x != r.x) || (y != r.y) || (width != r.width) || (height != r.height); }

        // Rectangle operations
        Vector2 Location() const;
        Vector2 Center() const;

        bool IsEmpty() const { return (width == 0 && height == 0 && x == 0 && y == 0); }

        bool Contains(int ix, int iy) const { return (x <= ix) && (ix < (x + width)) && (y <= iy) && (iy < (y + height)); }
        bool Contains(const Vector2& point) const;
        bool Contains(const Rectangle& r) const { return (x <= r.x) && ((r.x + r.width) <= (x + width)) && (y <= r.y) && ((r.y + r.height) <= (y + height)); }

        void Inflate(int horizAmount, int vertAmount);

        bool Intersects(const Rectangle& r) const { return (r.x < (x + width)) && (x < (r.x + r.width)) && (r.y < (y + height)) && (y < (r.y + r.height)); }

        void Offset(int ox, int oy) { x += ox; y += oy; }

        // Static functions
        static Rectangle Intersect(const Rectangle& ra, const Rectangle& rb);

        static Rectangle Union(const Rectangle& ra, const Rectangle& rb);

        // Constants
        static const Rectangle Empty;
    };

    struct Viewport
    {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        float minDepth = 0.0f;
        float maxDepth = 1.0f;

        Viewport() = default;
        constexpr Viewport(float x_, float y_, float width_, float height_, float minDepth_ = 0.0f, float maxDepth_ = 1.0f)
            : x(x_), y(y_), width(width_), height(height_), minDepth(minDepth_), maxDepth(maxDepth_) {}

        explicit Viewport(const Rectangle& rect)
            : x(static_cast<float>(rect.x)), y(static_cast<float>(rect.y))
            , width(static_cast<float>(rect.width)), height(static_cast<float>(rect.height))
            , minDepth(0.0f), maxDepth(1.0f) {}

        Viewport(const Viewport&) = default;
        Viewport& operator=(const Viewport&) = default;

        Viewport(Viewport&&) = default;
        Viewport& operator=(Viewport&&) = default;

        // Comparison operators
        bool operator == (const Viewport& rhs) const;
        bool operator != (const Viewport& rhs) const;

        // Assignment operators
        Viewport& operator= (const Rectangle& rect);

        /// Get aspect ratio.
        float GetAspectRatio() const;
    };
}

#include "../Math/Math.inl"

