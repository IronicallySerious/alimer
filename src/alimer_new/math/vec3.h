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

// Some idea and concept adopted/taken from filament: https://github.com/google/filament/blob/master/LICENSE

#pragma once

#include "math/vec2.h"

namespace alimer {
    namespace math {

        template <typename T>
        struct tvec3
        {
        public:
            typedef T value_type;
            typedef T& reference;
            typedef T const& const_reference;
            typedef size_t size_type;
            static constexpr size_t SIZE = 3;

            union {
                T data[SIZE];
                tvec2<T> xy;
                tvec2<T> st;
                tvec2<T> rg;
                struct {
                    union {
                        T x;
                        T s;
                        T r;
                    };
                    union {
                        struct { T y, z; };
                        struct { T t, p; };
                        struct { T g, b; };
                        tvec2<T> yz;
                        tvec2<T> tp;
                        tvec2<T> gb;
                    };
                };
            };

            constexpr tvec3() = default;
            tvec3(const tvec3 &) = default;

            // handles implicit conversion to a tvec4. must not be explicit.
            template<typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value >::type>
            constexpr tvec3(U v) : x(v), y(v), z(v) { }

            template<typename A, typename B, typename C>
            constexpr tvec3(A x, B y, C z) : x(x), y(y), z(z) { }

            template<typename U>
            explicit constexpr tvec3(const tvec3<U>& other) : x(other.x), y(other.y), z(other.z) { }

            inline constexpr size_type size() const { return SIZE; }

            // array access
            inline constexpr T const& operator[](size_type i) const {
                assert(i < SIZE);
                return data[i];
            }

            inline constexpr T& operator[](size_type i) {
                assert(i < SIZE);
                return data[i];
            }

            // cross product works only on vectors of size 3
            template <typename RT>
            friend inline constexpr T cross(const tvec3& u, const tvec3<RT>& v) {
                return tvec3(u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);
            }
        };

        using float3 = tvec3<float>;
        using double3 = tvec3<double>;
        using half3 = tvec3<half>;
        using int3 = tvec3<int32_t>;
        using uint3 = tvec3<uint32_t>;
        using short3 = tvec3<int16_t>;
        using ushort3 = tvec3<uint16_t>;
        using byte3 = tvec3<int8_t>;
        using ubyte3 = tvec3<uint8_t>;
        using bool3 = tvec3<bool>;

    } // namespace math
} // namespace alimer
