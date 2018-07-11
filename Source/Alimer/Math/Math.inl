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

// TODO: Add SIMD

namespace Alimer
{
    /****************************************************************************
    *
    * Rectangle
    *
    ****************************************************************************/

    inline Vector2 Rectangle::Location() const
    {
        return Vector2(static_cast<float>(x), static_cast<float>(y));
    }

    inline Vector2 Rectangle::Center() const
    {
        return Vector2(
            static_cast<float>(x) + static_cast<float>(width / 2.0f),
            static_cast<float>(y) + static_cast<float>(height / 2.0f));
    }

    inline bool Rectangle::Contains(const Vector2& point) const
    {
        return (static_cast<float>(x) <= point.x) && (point.x < static_cast<float>(x + width)) && (static_cast<float>(y) <= point.y) && (point.y < static_cast<float>(y + height));
    }

    inline void Rectangle::Inflate(int horizAmount, int vertAmount)
    {
        x -= horizAmount;
        y -= vertAmount;
        width += horizAmount;
        height += vertAmount;
    }

    inline Rectangle Rectangle::Intersect(const Rectangle& ra, const Rectangle& rb)
    {
        int righta = ra.x + ra.width;
        int rightb = rb.x + rb.width;

        int bottoma = ra.y + ra.height;
        int bottomb = rb.y + rb.height;

        int maxX = ra.x > rb.x ? ra.x : rb.x;
        int maxY = ra.y > rb.y ? ra.y : rb.y;

        int minRight = righta < rightb ? righta : rightb;
        int minBottom = bottoma < bottomb ? bottoma : bottomb;

        Rectangle result;

        if ((minRight > maxX) && (minBottom > maxY))
        {
            result.x = maxX;
            result.y = maxY;
            result.width = minRight - maxX;
            result.height = minBottom - maxY;
        }
        else
        {
            result.x = 0;
            result.y = 0;
            result.width = 0;
            result.height = 0;
        }

        return result;
    }

    inline Rectangle Rectangle::Union(const Rectangle& ra, const Rectangle& rb)
    {
        int righta = ra.x + ra.width;
        int rightb = rb.x + rb.width;

        int bottoma = ra.y + ra.height;
        int bottomb = rb.y + rb.height;

        int minX = ra.x < rb.x ? ra.x : rb.x;
        int minY = ra.y < rb.y ? ra.y : rb.y;

        int maxRight = righta > rightb ? righta : rightb;
        int maxBottom = bottoma > bottomb ? bottoma : bottomb;

        Rectangle result;
        result.x = minX;
        result.y = minY;
        result.width = maxRight - minX;
        result.height = maxBottom - minY;
        return result;
    }

    /****************************************************************************
    *
    * Viewport
    *
    ****************************************************************************/

    inline bool Viewport::operator == (const Viewport& rhs) const
    {
        return (x == rhs.x && y == rhs.y
            && width == rhs.width && height == rhs.height
            && minDepth == rhs.minDepth && maxDepth == rhs.maxDepth);
    }

    inline bool Viewport::operator != (const Viewport& rhs) const
    {
        return (x != rhs.x || y != rhs.y
            || width != rhs.width || height != rhs.height
            || minDepth != rhs.minDepth || maxDepth != rhs.maxDepth);
    }

    inline Viewport& Viewport::operator= (const Rectangle& rect)
    {
        x = static_cast<float>(rect.x);
        y = static_cast<float>(rect.y);
        width = static_cast<float>(rect.width);
        height = static_cast<float>(rect.height);
        minDepth = 0.0f; maxDepth = 1.0f;
        return *this;
    }

    inline float Viewport::GetAspectRatio() const
    {
        if (width == 0.0f || height == 0.0f)
            return 0.0f;

        return (width / height);
    }

}
