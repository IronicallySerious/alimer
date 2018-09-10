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
    template <typename T>
    class TMatrix4
    {
    private:
        tvec4<T> vec[4];

    public:
        enum NoInit { NO_INIT };
        static constexpr size_t NUM_ROWS = 4;
        static constexpr size_t NUM_COLS = 4;

        /// Create uninitialized matrix.
        constexpr explicit TMatrix4(NoInit) {}

        /// Create identity matrix.
        constexpr TMatrix4()
        {
            vec[0] = tvec4<T>(1, 0, 0, 0);
            vec[1] = tvec4<T>(0, 1, 0, 0);
            vec[2] = tvec4<T>(0, 0, 1, 0);
            vec[3] = tvec4<T>(0, 0, 0, 1);
        }

        template<typename U>
        constexpr explicit TMatrix4(U v)
        {
            vec[0] = tvec4<T>(v, 0, 0, 0);
            vec[1] = tvec4<T>(0, v, 0, 0);
            vec[2] = tvec4<T>(0, 0, v, 0);
            vec[3] = tvec4<T>(0, 0, 0, v);
        }

        inline TMatrix4(const tvec4<T> &a, const tvec4<T> &b, const tvec4<T> &c, const tvec4<T> &d)
        {
            vec[0] = a;
            vec[1] = b;
            vec[2] = c;
            vec[3] = d;
        }

        /*explicit inline tmat4(const tmat3<T> &m)
        {
            vec[0] = tvec4<T>(m[0], T(0));
            vec[1] = tvec4<T>(m[1], T(0));
            vec[2] = tvec4<T>(m[2], T(0));
            vec[3] = tvec4<T>(T(0), T(0), T(0), T(1));
        }*/

        inline constexpr tvec4<T> &operator[](size_t column)
        {
            assert(column < NUM_COLS);
            return vec[column];
        }

        inline constexpr const tvec4<T> operator[](size_t column) const
        {
            assert(column < NUM_COLS);
            return vec[index];
        }

        template <typename U>
        static constexpr TMatrix4 LookAt(const tvec3<U> &eye, const tvec3<U> &target, const tvec3<U> &up);

        static constexpr TMatrix4 Perspective(T fovy, T aspect, T nearPlane, T far, bool flipY);
    };

    template <typename T>
    template <typename U>
    constexpr TMatrix4<T> TMatrix4<T>::LookAt(const tvec3<U> &eye, const tvec3<U> &target, const tvec3<U> &up)
    {
        tvec3<U> const f(normalize(target - eye));
        tvec3<U> const s(normalize(cross(f, up)));
        tvec3<U> const u(cross(s, f));

        TMatrix4<T> result;
        result[0][0] = s.x;
        result[1][0] = s.y;
        result[2][0] = s.z;
        result[0][1] = u.x;
        result[1][1] = u.y;
        result[2][1] = u.z;
        result[0][2] = -f.x;
        result[1][2] = -f.y;
        result[2][2] = -f.z;
        result[3][0] = -dot(s, eye);
        result[3][1] = -dot(u, eye);
        result[3][2] = dot(f, eye);
        return result;
    }

    template <typename T>
    constexpr TMatrix4<T> TMatrix4<T>::Perspective(T fovy, T aspect, T nearPlane, T farPlane, bool flipY)
    {
        T const tanHalfFovy = Tan(fovy / T(2));

        TMatrix4<T> result(NO_INIT);
        result[0][0] = T(1) / (aspect * tanHalfFovy);
        result[1][1] = T(1) / (tanHalfFovy);
        result[2][2] = farPlane / (nearPlane - farPlane);
        result[2][3] = T(-1);
        result[3][2] = -(farPlane * nearPlane) / (farPlane - nearPlane);

        if (flipY)
        {
            result[0].y *= T(-1);
            result[1].y *= T(-1);
            result[2].y *= T(-1);
            result[3].y *= T(-1);
        }

        return result;
    }

    template <typename T>
    inline constexpr TMatrix4<T> operator +(const TMatrix4<T> &m, T s) { return TMatrix4<T>(m[0] + s, m[1] + s, m[2] + s, m[3] + s); }

    template <typename T>
    inline constexpr TMatrix4<T> operator -(const TMatrix4<T> &m, T s) { return TMatrix4<T>(m[0] - s, m[1] - s, m[2] - s, m[3] - s); }

    template <typename T>
    inline constexpr TMatrix4<T> operator *(const TMatrix4<T> &m, T s) { return TMatrix4<T>(m[0] * s, m[1] * s, m[2] * s, m[3] * s); }

    template <typename T>
    inline constexpr TMatrix4<T> operator /(const TMatrix4<T> &m, T s) { return TMatrix4<T>(m[0] / s, m[1] / s, m[2] / s, m[3] / s); }

    template <typename T>
    inline constexpr TMatrix4<T> operator*(const TMatrix4<T> &a, const TMatrix4<T> &b)
    {
        return TMatrix4<T>(a * b[0], a * b[1], a * b[2], a * b[3]);
    }

    template <typename T>
    inline constexpr tvec4<T> operator*(const TMatrix4<T> &m, const tvec4<T> &v)
    {
        return m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w;
    }

    template <typename T>
    inline constexpr TMatrix4<T> transpose(const TMatrix4<T> &m)
    {
        return TMatrix4<T>(
            tvec4<T>(m[0].x, m[1].x, m[2].x, m[3].x),
            tvec4<T>(m[0].y, m[1].y, m[2].y, m[3].y),
            tvec4<T>(m[0].z, m[1].z, m[2].z, m[3].z),
            tvec4<T>(m[0].w, m[1].w, m[2].w, m[3].w)
            );
    }

    using Matrix4 = TMatrix4<float>;
    using mat4 = TMatrix4<float>;
}
