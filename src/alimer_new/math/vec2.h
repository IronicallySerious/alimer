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

#include "math/half.h"

namespace alimer {
    namespace math {

        template <typename T>
        struct tvec2
        {
        public:
            typedef T value_type;
            typedef T& reference;
            typedef T const& const_reference;
            typedef size_t size_type;
            static constexpr size_t SIZE = 2;

            union {
                T data[SIZE];
                struct { T x, y; };
                struct { T s, t; };
                struct { T r, g; };
            };

            constexpr tvec2() = default;
            tvec2(const tvec2 &) = default;

            // handles implicit conversion to a tvec4. must not be explicit.
            template<typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value >::type>
            constexpr tvec2(U v) : x(v), y(v) { }

            template<typename A, typename B>
            constexpr tvec2(A x, B y) : x(x), y(y) { }

            template<typename U>
            explicit constexpr tvec2(const tvec2<U>& other) : x(other.x), y(other.y) { }

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

            // cross product works only on vectors of size 2 or 3
            template <typename RT>
            friend inline constexpr T cross(const tvec2& u, const tvec2<RT>& v) {
                return T(u.x*v.y - u.y*v.x);
            }

            static constexpr tvec2 zero() { return tvec2(T(0), T(0)); }
            static constexpr tvec2 one() { return tvec2(T(1), T(1)); }
            static constexpr tvec2 unit_x() { return tvec2(T(1), T(0)); }
            static constexpr tvec2 unit_y() { return tvec2(T(0), T(1)); }
        };

        using float2 = tvec2<float>;
        using double2 = tvec2<double>;
        using half2 = tvec2<half>;
        using int2 = tvec2<int32_t>;
        using uint2 = tvec2<uint32_t>;
        using short2 = tvec2<int16_t>;
        using ushort2 = tvec2<uint16_t>;
        using byte2 = tvec2<int8_t>;
        using ubyte2 = tvec2<uint8_t>;
        using bool2 = tvec2<bool>;

    } // namespace math
} // namespace alimer
