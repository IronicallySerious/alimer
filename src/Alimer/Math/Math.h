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

#include "AlimerConfig.h"
#include <stdint.h>
#include <cmath>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201 4203 4244 4702) 
#endif

namespace alimer
{
    template <typename T> struct tvec2;
    template <typename T> struct tvec3;
    template <typename T> struct tvec4;
    class mat2;
    class mat3;
    class mat4;
    template <typename T> struct trect;

    template <typename T> inline T pi() { return T(3.1415926535897932384626433832795028841971); }
    template <typename T> inline T half_pi() { return T(0.5) * pi<T>(); }
    template <typename T> inline T one_over_root_two() { return T(0.7071067811865476); }

    template <typename T>
    struct tvec2
    {
        union
        {
            T data[2];
            struct
            {
                T x, y;
            };
        };

        tvec2() = default;
        tvec2(const tvec2 &) = default;

        explicit inline tvec2(T v)
        {
            x = v;
            y = v;
        }

        template <typename U>
        explicit inline tvec2(const tvec2<U> &u)
        {
            x = T(u.x);
            y = T(u.y);
        }

        inline tvec2(T x, T y)
        {
            this->x = x;
            this->y = y;
        }

        inline T &operator[](size_t index)
        {
            return data[index];
        }

        inline const T &operator[](size_t index) const
        {
            return data[index];
        }

        static constexpr tvec2 zero() { return tvec2(0, 0); }
        static constexpr tvec2 one() { return tvec2(1, 1); }
        static constexpr tvec2 unit_x() { return tvec2(1, 0); }
        static constexpr tvec2 unit_y() { return tvec2(0, 1); }
    };

    template <typename T>
    struct tvec3
    {
        union
        {
            T data[3];
            struct
            {
                T x, y, z;
            };
        };

        tvec3() = default;
        tvec3(const tvec3 &) = default;

        template <typename U>
        explicit inline tvec3(const tvec3<U> &u)
        {
            x = T(u.x);
            y = T(u.y);
            z = T(u.z);
        }

        inline tvec3(const tvec2<T> &a, float b)
        {
            x = a.x;
            y = a.y;
            z = b;
        }

        inline tvec3(float a, const tvec2<T> &b)
        {
            x = a;
            y = b.x;
            z = b.y;
        }

        explicit inline tvec3(T v)
        {
            x = v;
            y = v;
            z = v;
        }

        inline tvec3(T x, T y, T z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }

        inline T const& operator[](size_t index) const
        {
            return data[index];
        }

        inline T& operator[](size_t index)
        {
            return data[index];
        }

        static constexpr tvec3 zero() { return tvec3(0, 0, 0); }
        static constexpr tvec3 one() { return tvec3(1, 1, 1); }
        static constexpr tvec3 unit_x() { return tvec3(1, 0, 0); }
        static constexpr tvec3 unit_y() { return tvec3(0, 1, 0); }
        static constexpr tvec3 unit_z() { return tvec3(0, 0, 1); }
        static constexpr tvec3 up() { return tvec3(0, 1, 0); }
        static constexpr tvec3 down() { return tvec3(0, -1, 0); }
        static constexpr tvec3 right() { return tvec3(1, 0, 0); }
        static constexpr tvec3 left() { return tvec3(-1, 0, 0); }
        static constexpr tvec3 forward() { return tvec3(0, 0, -1); }
        static constexpr tvec3 backward() { return tvec3(0, 0, 1); }
    };

    template <typename T>
    struct tvec4
    {
        union
        {
            T data[4];
            struct
            {
                T x, y, z, w;
            };
        };

        tvec4() = default;
        tvec4(const tvec4 &) = default;

        template <typename U>
        explicit inline tvec4(const tvec4<U> &u)
        {
            x = T(u.x);
            y = T(u.y);
            z = T(u.z);
            w = T(u.w);
        }

        inline tvec4(const tvec2<T> &a, const tvec2<T> &b)
        {
            x = a.x;
            y = a.y;
            z = b.x;
            w = b.y;
        }

        inline tvec4(const tvec3<T> &a, T b)
        {
            x = a.x;
            y = a.y;
            z = a.z;
            w = b;
        }

        inline tvec4(T a, const tvec3<T> &b)
        {
            x = a;
            y = b.x;
            z = b.y;
            w = b.z;
        }

        inline tvec4(const tvec2<T> &a, T b, T c)
        {
            x = a.x;
            y = a.y;
            z = b;
            w = c;
        }

        inline tvec4(T a, const tvec2<T> &b, T c)
        {
            x = a;
            y = b.x;
            z = b.y;
            w = c;
        }

        inline tvec4(T a, T b, const tvec2<T> &c)
        {
            x = a;
            y = b;
            z = c.x;
            w = c.y;
        }

        explicit inline tvec4(T v)
        {
            x = v;
            y = v;
            z = v;
            w = v;
        }

        inline tvec4(T x, T y, T z, T w)
        {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }

        inline T &operator[](size_t index)
        {
            return data[index];
        }

        inline const T &operator[](size_t index) const
        {
            return data[index];
        }

        static constexpr tvec4 zero() { return tvec4(0); }
        static constexpr tvec4 one() { return tvec4(1); }
        static constexpr tvec4 unit_x() { return tvec4(1, 0, 0, 0); }
        static constexpr tvec4 unit_y() { return tvec4(0, 1, 0, 0); }
        static constexpr tvec4 unit_z() { return tvec4(0, 0, 1, 0); }
        static constexpr tvec4 unit_w() { return tvec4(0, 0, 0, 1); }

        inline constexpr tvec3<T> xyz() const { return tvec3<T>(x, y, z); }
    };

    class ALIMER_API mat2
    {
    public:
        enum no_init { NO_INIT };

        explicit inline mat2(no_init) {}

        inline mat2()
        {
            data[0] = tvec2<float>(1.0f, 0.0f);
            data[1] = tvec2<float>(0.0f, 1.0f);
        }

        explicit inline mat2(float v)
        {
            data[0] = tvec2<float>(v, 0.0f);
            data[1] = tvec2<float>(0.0f, v);
        }

        inline mat2(const tvec2<float> &a, const tvec2<float> &b)
        {
            data[0] = a;
            data[1] = b;
        }

        inline tvec2<float> &operator[](size_t index)
        {
            return data[index];
        }

        inline const tvec2<float> &operator[](size_t index) const
        {
            return data[index];
        }

    private:
        tvec2<float> data[2];
    };

    class ALIMER_API mat3
    {
    public:
        enum no_init { NO_INIT };

        explicit inline mat3(no_init) {}

        inline mat3()
        {
            data[0] = tvec3<float>(1.0f, 0.0f, 0.0f);
            data[1] = tvec3<float>(0.0f, 1.0f, 0.0f);
            data[2] = tvec3<float>(0.0f, 0.0f, 1.0f);
        }

        explicit inline mat3(float v)
        {
            data[0] = tvec3<float>(v, 0.0f, 0.0f);
            data[1] = tvec3<float>(0.0f, v, 0.0f);
            data[2] = tvec3<float>(0.0f, 0.0f, v);
        }

        inline mat3(const tvec3<float> &a, const tvec3<float> &b, const tvec3<float> &c)
        {
            data[0] = a;
            data[1] = b;
            data[2] = c;
        }

        inline tvec3<float> &operator[](size_t index)
        {
            return data[index];
        }

        inline const tvec3<float> &operator[](size_t index) const
        {
            return data[index];
        }

        static inline mat3 identity() { return mat3(1); }

    private:
        tvec3<float> data[3];
    };

    class ALIMER_API mat4
    {
    public:
        enum no_init { NO_INIT };

        explicit inline mat4(no_init) {}

        inline mat4()
        {
            data[0] = tvec4<float>(1.0f, 0.0f, 0.0f, 0.0f);
            data[1] = tvec4<float>(0.0f, 1.0f, 0.0f, 0.0f);
            data[2] = tvec4<float>(0.0f, 0.0f, 1.0f, 0.0f);
            data[3] = tvec4<float>(0.0f, 0.0f, 0.0f, 1.0f);
        }

        explicit inline mat4(float v)
        {
            data[0] = tvec4<float>(v, 0.0f, 0.0f, 0.0f);
            data[1] = tvec4<float>(0.0f, v, 0.0f, 0.0f);
            data[2] = tvec4<float>(0.0f, 0.0f, v, 0.0f);
            data[3] = tvec4<float>(0.0f, 0.0f, 0.0f, v);
        }

        explicit inline mat4(const mat3 &m)
        {
            data[0] = tvec4<float>(m[0], 0.0f);
            data[1] = tvec4<float>(m[1], 0.0f);
            data[2] = tvec4<float>(m[2], 0.0f);
            data[3] = tvec4<float>(0.0f, 0.0f, 0.0f, 1.0f);
        }

        inline mat4(const tvec4<float> &a, const tvec4<float> &b, const tvec4<float> &c, const tvec4<float> &d)
        {
            data[0] = a;
            data[1] = b;
            data[2] = c;
            data[3] = d;
        }

        inline const tvec4<float>& operator[](size_t index) const
        {
            return data[index];
        }

        inline tvec4<float>& operator[](size_t index)
        {
            return data[index];
        }

        static inline mat4 zero() { return mat4(0.0f); }
        static inline mat4 identity() { return mat4(1.0f); }

        static inline mat4 translate(const tvec3<float>& v)
        {
            return mat4(
                tvec4<float>(1.0f, 0.0f, 0.0f, 0.0f),
                tvec4<float>(0.0f, 1.0f, 0.0f, 0.0f),
                tvec4<float>(0.0f, 0.0f, 1.0f, 0.0f),
                tvec4<float>(v, 1.0f)
            );
        }

        static inline mat4 scale(const tvec3<float>& v)
        {
            return mat4(
                tvec4<float>(v.x, 0.0f, 0.0f, 0.0f),
                tvec4<float>(0.0f, v.y, 0.0f, 0.0f),
                tvec4<float>(0.0f, 0.0f, v.z, 0.0f),
                tvec4<float>(0.0f, 0.0f, 0.0f, 1.0f)
            );
        }

        static mat4 perspective(float fovy, float aspect, float zNear, float zFar);

    private:
        tvec4<float> data[4];
    };

    struct quat : private tvec4<float>
    {
        quat() = default;
        quat(const quat &) = default;
        quat(float w, float x, float y, float z)
            : tvec4<float>(x, y, z, w)
        {}

        explicit inline quat(const tvec4<float> &v)
            : tvec4<float>(v)
        {}

        inline quat(float w, const tvec3<float> &v)
            : tvec4<float>(v, w)
        {}

        inline const tvec4<float> &as_vec4() const
        {
            return *static_cast<const tvec4<float>*>(this);
        }

        using tvec4<float>::x;
        using tvec4<float>::y;
        using tvec4<float>::z;
        using tvec4<float>::w;

        static inline quat zero() { return quat(0.0f, 0.0f, 0.0f, 0.0f); }
        static inline quat identity() { return quat(0.0f, 0.0f, 0.0f, 1.0f); }
    };

    using uint = uint32_t;
    using vec2 = tvec2<float>;
    using vec3 = tvec3<float>;
    using vec4 = tvec4<float>;

    using ivec2 = tvec2<int32_t>;
    using ivec3 = tvec3<int32_t>;
    using ivec4 = tvec4<int32_t>;
    using uvec2 = tvec2<uint32_t>;
    using uvec3 = tvec3<uint32_t>;
    using uvec4 = tvec4<uint32_t>;

    using u16vec2 = tvec2<uint16_t>;
    using u16vec3 = tvec3<uint16_t>;
    using u16vec4 = tvec4<uint16_t>;
    using i16vec2 = tvec2<int16_t>;
    using i16vec3 = tvec3<int16_t>;
    using i16vec4 = tvec4<int16_t>;

    using u8vec2 = tvec2<uint8_t>;
    using u8vec3 = tvec3<uint8_t>;
    using u8vec4 = tvec4<uint8_t>;
    using i8vec2 = tvec2<int8_t>;
    using i8vec3 = tvec3<int8_t>;
    using i8vec4 = tvec4<int8_t>;

    using bvec2 = tvec2<bool>;
    using bvec3 = tvec3<bool>;
    using bvec4 = tvec4<bool>;

    // dot
    inline float dot(const vec2 &a, const vec2 &b) { return a.x * b.x + a.y * b.y; }
    inline float dot(const vec3 &a, const vec3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
    inline float dot(const vec4 &a, const vec4 &b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

    // min, max, clamp
    //template <typename T> T min(T a, T b) { return b < a ? b : a; }
    //template <typename T> T max(T a, T b) { return a < b ? b : a; }
    template <typename T> T clamp(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }
    template <typename T> T sign(T v) { return v < T(0) ? T(-1) : (v > T(0) ? T(1) : T(0)); }
    template <typename T> T sin(T v) { return std::sin(v); }
    template <typename T> T cos(T v) { return std::cos(v); }
    template <typename T> T tan(T v) { return std::tan(v); }
    template <typename T> T asin(T v) { return std::asin(v); }
    template <typename T> T acos(T v) { return std::acos(v); }
    template <typename T> T atan(T v) { return std::atan(v); }
    template <typename T> T log2(T v) { return std::log2(v); }
    template <typename T> T log10(T v) { return std::log10(v); }
    template <typename T> T log(T v) { return std::log(v); }
    template <typename T> T exp2(T v) { return std::exp2(v); }
    template <typename T> T exp(T v) { return std::exp(v); }
    template <typename T> T pow(T a, T b) { return std::pow(a, b); }

    // arithmetic operations
#define MUGLM_DEFINE_ARITH_OP(op) \
template <typename T> inline tvec2<T> operator op(const tvec2<T> &a, const tvec2<T> &b) { return tvec2<T>(a.x op b.x, a.y op b.y); } \
template <typename T> inline tvec3<T> operator op(const tvec3<T> &a, const tvec3<T> &b) { return tvec3<T>(a.x op b.x, a.y op b.y, a.z op b.z); } \
template <typename T> inline tvec4<T> operator op(const tvec4<T> &a, const tvec4<T> &b) { return tvec4<T>(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w); } \
template <typename T> inline tvec2<T> operator op(const tvec2<T> &a, T b) { return tvec2<T>(a.x op b, a.y op b); } \
template <typename T> inline tvec3<T> operator op(const tvec3<T> &a, T b) { return tvec3<T>(a.x op b, a.y op b, a.z op b); } \
template <typename T> inline tvec4<T> operator op(const tvec4<T> &a, T b) { return tvec4<T>(a.x op b, a.y op b, a.z op b, a.w op b); } \
template <typename T> inline tvec2<T> operator op(T a, const tvec2<T> &b) { return tvec2<T>(a op b.x, a op b.y); } \
template <typename T> inline tvec3<T> operator op(T a, const tvec3<T> &b) { return tvec3<T>(a op b.x, a op b.y, a op b.z); } \
template <typename T> inline tvec4<T> operator op(T a, const tvec4<T> &b) { return tvec4<T>(a op b.x, a op b.y, a op b.z, a op b.w); }
    MUGLM_DEFINE_ARITH_OP(+);
    MUGLM_DEFINE_ARITH_OP(-);
    MUGLM_DEFINE_ARITH_OP(*);
    MUGLM_DEFINE_ARITH_OP(/);
    MUGLM_DEFINE_ARITH_OP(^);
    MUGLM_DEFINE_ARITH_OP(&);
    MUGLM_DEFINE_ARITH_OP(|);
    MUGLM_DEFINE_ARITH_OP(>>);
    MUGLM_DEFINE_ARITH_OP(<<);

#define MUGLM_DEFINE_BOOL_OP(bop, op) \
template <typename T> inline bvec2 bop(const tvec2<T> &a, const tvec2<T> &b) { return bvec2(a.x op b.x, a.y op b.y); } \
template <typename T> inline bvec3 bop(const tvec3<T> &a, const tvec3<T> &b) { return bvec3(a.x op b.x, a.y op b.y, a.z op b.z); } \
template <typename T> inline bvec4 bop(const tvec4<T> &a, const tvec4<T> &b) { return bvec4(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w); }
    MUGLM_DEFINE_BOOL_OP(notEqual, != )
        MUGLM_DEFINE_BOOL_OP(equal, == )
        MUGLM_DEFINE_BOOL_OP(lessThan, < )
        MUGLM_DEFINE_BOOL_OP(lessThanEqual, <= )
        MUGLM_DEFINE_BOOL_OP(greaterThan, > )
        MUGLM_DEFINE_BOOL_OP(greaterThanEqual, >= )

        inline bool any(const bvec2 &v) { return v.x || v.y; }
    inline bool any(const bvec3 &v) { return v.x || v.y || v.z; }
    inline bool any(const bvec4 &v) { return v.x || v.y || v.z || v.w; }
    inline bool all(const bvec2 &v) { return v.x && v.y; }
    inline bool all(const bvec3 &v) { return v.x && v.y && v.z; }
    inline bool all(const bvec4 &v) { return v.x && v.y && v.z && v.w; }

    template <typename T> inline tvec2<T> operator-(const tvec2<T> &v) { return tvec2<T>(-v.x, -v.y); }
    template <typename T> inline tvec3<T> operator-(const tvec3<T> &v) { return tvec3<T>(-v.x, -v.y, -v.z); }
    template <typename T> inline tvec4<T> operator-(const tvec4<T> &v) { return tvec4<T>(-v.x, -v.y, -v.z, -v.w); }
    template <typename T> inline tvec2<T> operator~(const tvec2<T> &v) { return tvec2<T>(~v.x, ~v.y); }
    template <typename T> inline tvec3<T> operator~(const tvec3<T> &v) { return tvec3<T>(~v.x, ~v.y, ~v.z); }
    template <typename T> inline tvec4<T> operator~(const tvec4<T> &v) { return tvec4<T>(~v.x, ~v.y, ~v.z, ~v.w); }

#define MUGLM_VECTORIZED_FUNC1(func) \
template <typename T> inline tvec2<T> func(const tvec2<T> &a) { return tvec2<T>(func(a.x), func(a.y)); } \
template <typename T> inline tvec3<T> func(const tvec3<T> &a) { return tvec3<T>(func(a.x), func(a.y), func(a.z)); } \
template <typename T> inline tvec4<T> func(const tvec4<T> &a) { return tvec4<T>(func(a.x), func(a.y), func(a.z), func(a.w)); }

#define MUGLM_VECTORIZED_FUNC2(func) \
template <typename T> inline tvec2<T> func(const tvec2<T> &a, const tvec2<T> &b) { return tvec2<T>(func(a.x, b.x), func(a.y, b.y)); } \
template <typename T> inline tvec3<T> func(const tvec3<T> &a, const tvec3<T> &b) { return tvec3<T>(func(a.x, b.x), func(a.y, b.y), func(a.z, b.z)); } \
template <typename T> inline tvec4<T> func(const tvec4<T> &a, const tvec4<T> &b) { return tvec4<T>(func(a.x, b.x), func(a.y, b.y), func(a.z, b.z), func(a.w, b.w)); }

#define MUGLM_VECTORIZED_FUNC3(func) \
template <typename T> inline tvec2<T> func(const tvec2<T> &a, const tvec2<T> &b, const tvec2<T> &c) { return tvec2<T>(func(a.x, b.x, c.x), func(a.y, b.y, c.y)); } \
template <typename T> inline tvec3<T> func(const tvec3<T> &a, const tvec3<T> &b, const tvec3<T> &c) { return tvec3<T>(func(a.x, b.x, c.x), func(a.y, b.y, c.y), func(a.z, b.z, c.z)); } \
template <typename T> inline tvec4<T> func(const tvec4<T> &a, const tvec4<T> &b, const tvec4<T> &c) { return tvec4<T>(func(a.x, b.x, c.x), func(a.y, b.y, c.y), func(a.z, b.z, c.z), func(a.w, b.w, c.w)); }

    MUGLM_VECTORIZED_FUNC1(sign)
        MUGLM_VECTORIZED_FUNC1(sin)
        MUGLM_VECTORIZED_FUNC1(cos)
        MUGLM_VECTORIZED_FUNC1(tan)
        MUGLM_VECTORIZED_FUNC1(asin)
        MUGLM_VECTORIZED_FUNC1(acos)
        MUGLM_VECTORIZED_FUNC1(atan)
        MUGLM_VECTORIZED_FUNC1(log2)
        MUGLM_VECTORIZED_FUNC1(log10)
        MUGLM_VECTORIZED_FUNC1(log)
        MUGLM_VECTORIZED_FUNC1(exp2)
        MUGLM_VECTORIZED_FUNC1(exp)
        //MUGLM_VECTORIZED_FUNC2(min)
        //MUGLM_VECTORIZED_FUNC2(max)
        MUGLM_VECTORIZED_FUNC2(pow)
        MUGLM_VECTORIZED_FUNC3(clamp)

        // mix
        template <typename T, typename Lerp> inline T mix(const T &a, const T &b, const Lerp &lerp) { return a + (b - a) * lerp; }

    // cross
    inline vec3 cross(const vec3 &a, const vec3 &b) { return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }

    // matrix multiply
    inline tvec2<float> operator*(const mat2 &m, const tvec2<float>& v)
    {
        return m[0] * v.x + m[1] * v.y;
    }

    inline tvec3<float> operator*(const mat3 &m, const tvec3<float>& v)
    {
        return m[0] * v.x + m[1] * v.y + m[2] * v.z;
    }

    inline mat2 operator*(const mat2 &a, const mat2& b)
    {
        return mat2(a * b[0], a * b[1]);
    }

    inline mat3 operator*(const mat3 &a, const mat3& b)
    {
        return mat3(a * b[0], a * b[1], a * b[2]);
    }

    // mat4 operators
    inline mat4 operator +(const mat4& m, float s)
    {
        return mat4(m[0] + s, m[1] + s, m[2] + s, m[3] + s);
    }

    inline mat4 operator -(const mat4& m, float s)
    {
        return mat4(m[0] - s, m[1] - s, m[2] - s, m[3] - s);
    }

    inline mat4 operator *(const mat4& m, float s)
    {
        return mat4(m[0] * s, m[1] * s, m[2] * s, m[3] * s);
    }

    inline mat4 operator /(const mat4& m, float s)
    {
        return mat4(m[0] / s, m[1] / s, m[2] / s, m[3] / s);
    }

    inline vec4 operator*(const mat4& m, const vec4& v)
    {
        return m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w;
    }

    inline mat4 operator*(const mat4& a, const mat4& b)
    {
        return mat4(a * b[0], a * b[1], a * b[2], a * b[3]);
    }

    inline mat3 mat3_cast(const mat4 &m)
    {
        mat3 result(mat3::NO_INIT);
        for (size_t col = 0; col < 3; col++)
        {
            for (size_t row = 0; row < 3; row++)
            {
                result[col][row] = m[col][row];
            }
        }
        return result;
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
