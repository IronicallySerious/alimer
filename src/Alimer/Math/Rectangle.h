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

namespace alimer
{
    /// Defines a 2D integer rectangle.
    class ALIMER_API Rectangle
    {
    public:
        /// The x-coordinate of the rectangle.
        int x;
        /// The y-coordinate of the rectangle.
        int y;
        /// The width of the rectangle.
        int width;
        /// The height of the rectangle.
        int height;

        /// Construct an identity matrix.
        Rectangle()  noexcept : x(0), y(0), width(0), height(0) {}
        constexpr Rectangle(int width_, int height_) : x(0), y(0), width(width_), height(height_) {}
        constexpr Rectangle(int x_, int y_, int width_, int height_) : x(x_), y(y_), width(width_), height(height_) {}

        Rectangle(const Rectangle&) = default;
        Rectangle& operator=(const Rectangle&) = default;

        Rectangle(Rectangle&&) = default;
        Rectangle& operator=(Rectangle&&) = default;

        // Comparison operators
        bool operator == (const Rectangle& rhs) const { return (x == rhs.x) && (y == rhs.y) && (width == rhs.width) && (height == rhs.height); }
        bool operator != (const Rectangle& rhs) const { return (x != rhs.x) || (y != rhs.y) || (width != rhs.width) || (height != rhs.height); }

        bool IsEmpty() const { return (width == 0 && height == 0 && x == 0 && y == 0); }

        void Offset(int offsetX, int offsetY) { x += offsetX; y += offsetY; }

        /// Return left coordinate.
        int Left() const { return x; }

        /// Return top coordinate.
        int Top() const { return y; }

        /// Return right coordinate.
        int Right() const { return x + width; }

        /// Return bottom coordinate.
        int Bottom() const { return y + height; }

        // Rectangle operations
        IntVector2 Location() const { return IntVector2(x, y); }
        IntVector2 TopLeft() const { return IntVector2(x, y); }
        IntVector2 TopRight() const { return IntVector2(x + width, y); }
        IntVector2 BottomRight() const { return IntVector2(x + width, y + height); }
        IntVector2 BottomLeft() const { return IntVector2(x, y + height); }
        IntVector2 Center() const { return IntVector2(x + (width / 2), y + (height / 2)); }
        IntVector2 Size() const { return IntVector2(width, height); }
        IntVector2 HalfSize() const { return IntVector2(width / 2, height / 2); }
        float AspectRatio() const { return width / static_cast<float>(height); }

        /// Return as string.
        std::string ToString() const;

        // Constants
        static const Rectangle Empty;
    };

    /// Defines a 2D floating-point rectangle.
    class ALIMER_API RectangleF
    {
    public:
        /// The x-coordinate of the rectangle.
        float x;
        /// The y-coordinate of the rectangle.
        float y;
        /// The width of the rectangle.
        float width;
        /// The height of the rectangle.
        float height;

        /// Construct an identity matrix.
        RectangleF()  noexcept : x(0.0f), y(0.0f), width(0.0f), height(0.0f) {}
        constexpr RectangleF(float width_, float height_) : x(0.0f), y(0.0f), width(width_), height(height_) {}
        constexpr RectangleF(float x_, float y_, float width_, float height_) : x(x_), y(y_), width(width_), height(height_) {}
        constexpr RectangleF(uint32_t width_, uint32_t height_) : x(0.0f), y(0.0f), width(static_cast<float>(width_)), height(static_cast<float>(height_)) {}

        RectangleF(const RectangleF&) = default;
        RectangleF& operator=(const RectangleF&) = default;

        RectangleF(RectangleF&&) = default;
        RectangleF& operator=(RectangleF&&) = default;

        // Comparison operators
        bool operator == (const RectangleF& rhs) const { return (x == rhs.x) && (y == rhs.y) && (width == rhs.width) && (height == rhs.height); }
        bool operator != (const RectangleF& rhs) const { return (x != rhs.x) || (y != rhs.y) || (width != rhs.width) || (height != rhs.height); }

        bool IsEmpty() const { return (width == 0 && height == 0 && x == 0 && y == 0); }

        void Offset(float offsetX, float offsetY) { x += offsetX; y += offsetY; }

        /// Return left coordinate.
        float Left() const { return x; }

        /// Return top coordinate.
        float Top() const { return y; }

        /// Return right coordinate.
        float Right() const { return x + width; }

        /// Return bottom coordinate.
        float Bottom() const { return y + height; }

        // Rectangle operations
        Vector2 Location() const { return Vector2(x, y); }
        Vector2 TopLeft() const { return Vector2(x, y); }
        Vector2 TopRight() const { return Vector2(x + width, y); }
        Vector2 BottomRight() const { return Vector2(x + width, y + height); }
        Vector2 BottomLeft() const { return Vector2(x, y + height); }
        Vector2 Center() const { return Vector2(x + (width / 2.0f), y + (height / 2.0f)); }
        Vector2 Size() const { return Vector2(width, height); }
        Vector2 HalfSize() const { return Vector2(width / 2.0f, height / 2.0f); }
        float AspectRatio() const { return width / height; }

        /// Return as string.
        std::string ToString() const;

        // Constants
        static const RectangleF Empty;
    };
}
