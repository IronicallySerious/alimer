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

#include "../Math/Math.h"
#include "../Math/MathUtil.h"
#include <string>

namespace Alimer
{
    /// A 2x2 column-major floating-point matrix class.
    class ALIMER_API Matrix2
    {
    private:
        vec2 vec[2];

    public:
        enum NoInit { NO_INIT };

        /// Create uninitialized matrix.
        explicit inline Matrix2(NoInit) {}

        /// Create identity matrix.
        inline Matrix2()
        {
            vec[0] = vec2(1.0f, 0.0f);
            vec[1] = vec2(0.0f, 1.0f);
        }
        
        explicit inline Matrix2(float v)
        {
            vec[0] = vec2(v, 0.0f);
            vec[1] = vec2(0.0f, v);
        }

        inline Matrix2(const vec2 &a, const vec2 &b)
        {
            vec[0] = a;
            vec[1] = b;
        }

        inline constexpr vec2 &operator[](size_t column)
        {
            assert(column < 2);
            return vec[column];
        }

        inline constexpr const vec2 &operator[](size_t column) const
        {
            assert(column < 2);
            return vec[column];
        }

        Matrix2 Transpose() const;
        void Transpose(Matrix2* result) const;

        Matrix2 Invert() const;
        void Invert(Matrix2* result) const;
    };

    inline Matrix2 operator +(const Matrix2 &m, float s) { return Matrix2(m[0] + s, m[1] + s); }

    inline Matrix2 operator -(const Matrix2 &m, float s) { return Matrix2(m[0] - s, m[1] - s); }

    inline Matrix2 operator *(const Matrix2 &m, float s) { return Matrix2(m[0] * s, m[1] * s); }

    inline Matrix2 operator /(const Matrix2 &m, float s) { return Matrix2(m[0] / s, m[1] / s); }

    inline vec2 operator*(const Matrix2 &m, const vec2 &v)
    {
        return m[0] * v.x + m[1] * v.y;
    }

    inline Matrix2 operator*(const Matrix2 &a, const Matrix2 &b)
    {
        return Matrix2(a * b[0], a * b[1]);
    }

    inline Matrix2 Matrix2::Transpose() const
    {
        return Matrix2(vec2(vec[0].x, vec[1].x), vec2(vec[0].y, vec[1].y));
    }

    inline void Matrix2::Transpose(Matrix2* result) const
    {
        ALIMER_ASSERT(result);
        result->vec[0].x = vec[0].x;
        result->vec[0].y = vec[1].x;
        result->vec[1].x = vec[0].y;
        result->vec[1].y = vec[1].y; 
    }

    inline Matrix2 Matrix2::Invert() const
    {
        Matrix2 result;
        Invert(&result);
        return result;
    }

    inline void Matrix2::Invert(Matrix2* result) const
    {
        const float a = vec[0].x;
        const float c = vec[0].y;
        const float b = vec[1].x;
        const float d = vec[1].y;

        const float det((a * d) - (b * c));
        result->vec[0].x = d / det;
        result->vec[0].y = -c / det;
        result->vec[1].x = -b / det;
        result->vec[1].y = a / det;
    }

    // GLSL/HLSL style
    using mat2 = Matrix2;
    using float2x2 = Matrix2;
}
