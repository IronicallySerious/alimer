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
    /// A 4x4 column-major floating-point matrix class.
    class ALIMER_API Matrix4
    {
    private:
        tvec4<float> _values[4];

    public:
        enum NoInit { NO_INIT };
        static constexpr size_t NUM_ROWS = 4;
        static constexpr size_t NUM_COLS = 4;

        /// Create uninitialized matrix.
        explicit inline Matrix4(NoInit) {}

        /// Create identity matrix.
        inline Matrix4()
        {
            _values[0] = vec4(1.0f, 0.0f, 0.0f, 0.0f);
            _values[1] = vec4(0.0f, 1.0f, 0.0f, 0.0f);
            _values[2] = vec4(0.0f, 0.0f, 1.0f, 0.0f);
            _values[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }

        explicit inline Matrix4(float v)
        {
            _values[0] = vec4(v, 0.0f, 0.0f, 0.0f);
            _values[1] = vec4(0.0f, v, 0.0f, 0.0f);
            _values[2] = vec4(0.0f, 0.0f, v, 0.0f);
            _values[3] = vec4(0.0f, 0.0f, 0.0f, v);
        }

        inline Matrix4(const vec4 &a, const vec4 &b, const vec4 &c, const vec4 &d)
        {
            _values[0] = a;
            _values[1] = b;
            _values[2] = c;
            _values[3] = d;
        }

        /*explicit inline tmat4(const tmat3<T> &m)
        {
            _values[0] = tvec4<T>(m[0], T(0));
            _values[1] = tvec4<T>(m[1], T(0));
            _values[2] = tvec4<T>(m[2], T(0));
            _values[3] = tvec4<T>(T(0), T(0), T(0), T(1));
        }*/

        inline vec4 &operator[](size_t column)
        {
            assert(column < NUM_COLS);
            return _values[column];
        }

        inline const vec4 operator[](size_t column) const
        {
            assert(column < NUM_COLS);
            return _values[column];
        }

        static inline Matrix4 LookAt(const vec3 &eye, const vec3 &target, const vec3 &up);

        static inline Matrix4 Perspective(float fovy, float aspect, float nearPlane, float farPlane, bool flipY);
    };

    inline Matrix4 Matrix4::LookAt(const vec3 &eye, const vec3 &target, const vec3 &up)
    {
        vec3 const f(normalize(target - eye));
        vec3 const s(normalize(cross(f, up)));
        vec3 const u(cross(s, f));

        Matrix4 result;
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

    inline Matrix4 Matrix4::Perspective(float fovy, float aspect, float nearPlane, float farPlane, bool flipY)
    {
        float const tanHalfFovy = Tan(fovy / 2.0f);

        Matrix4 result(NO_INIT);
        result[0][0] = 1.0f / (aspect * tanHalfFovy);
        result[1][1] = 1.0f / (tanHalfFovy);
        result[2][2] = farPlane / (nearPlane - farPlane);
        result[2][3] = -1.0f;
        result[3][2] = -(farPlane * nearPlane) / (farPlane - nearPlane);

        if (flipY)
        {
            result[0].y *= -1.0f;
            result[1].y *= -1.0f;
            result[2].y *= -1.0f;
            result[3].y *= -1.0f;
        }

        return result;
    }

    inline Matrix4 operator +(const Matrix4 &m, float s) { return Matrix4(m[0] + s, m[1] + s, m[2] + s, m[3] + s); }

    inline Matrix4 operator -(const Matrix4 &m, float s) { return Matrix4(m[0] - s, m[1] - s, m[2] - s, m[3] - s); }

    inline Matrix4 operator *(const Matrix4 &m, float s) { return Matrix4(m[0] * s, m[1] * s, m[2] * s, m[3] * s); }

    inline Matrix4 operator /(const Matrix4 &m, float s) { return Matrix4(m[0] / s, m[1] / s, m[2] / s, m[3] / s); }

    //inline Matrix4 operator*(const Matrix4 &a, const Matrix4 &b)
    //{
    //    return Matrix4(a * b[0], a * b[1], a * b[2], a * b[3]);
    //}

    //inline vec4 operator*(const Matrix4 &m, const vec4 &v)
    //{
    //    return m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w;
    //}

    inline Matrix4 Transpose(const Matrix4 &m)
    {
        return Matrix4(
            vec4(m[0].x, m[1].x, m[2].x, m[3].x),
            vec4(m[0].y, m[1].y, m[2].y, m[3].y),
            vec4(m[0].z, m[1].z, m[2].z, m[3].z),
            vec4(m[0].w, m[1].w, m[2].w, m[3].w)
            );
    }

    // GLSL/HLSL style
    using mat4 = Matrix4;
    using float4x4 = Matrix4;
}
