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

#include "../Base/String.h"
#include "../Math/Math.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201 4203 4244 4702) 
#endif

namespace alimer
{
    /// Represents a 32-bit color (4 bytes) in BGRA order.
    struct ColorBGRA
    {
        union
        {
            struct
            {
                uint8_t b;
                uint8_t g;
                uint8_t r;
                uint8_t a;
            };
            uint32_t packedValue;
        };

        ColorBGRA() = default;

        ColorBGRA(const ColorBGRA&) = default;
        ColorBGRA& operator=(const ColorBGRA&) = default;

        ColorBGRA(ColorBGRA&&) = default;
        ColorBGRA& operator=(ColorBGRA&&) = default;

        constexpr ColorBGRA(uint32_t packedValue_) : packedValue(packedValue_) {}
        //ColorBGRA(float _r, float _g, float _b, float _a);
        //explicit ColorBGRA(_In_reads_(4) const float *pArray);

        operator uint32_t () const { return packedValue; }

        ColorBGRA& operator= (const uint32_t packedValue_) { packedValue = packedValue_; return *this; }
    };

    /// Represents a 32-bit color (4 bytes) in RGBA order.
    struct ColorRGBA
    {
        union
        {
            struct
            {
                uint8_t r;
                uint8_t g;
                uint8_t b;
                uint8_t a;
            };
            uint32_t packedValue;
        };

        ColorRGBA() = default;

        ColorRGBA(const ColorRGBA&) = default;
        ColorRGBA& operator=(const ColorRGBA&) = default;

        ColorRGBA(ColorRGBA&&) = default;
        ColorRGBA& operator=(ColorRGBA&&) = default;

        constexpr ColorRGBA(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) : r(r_), g(g_), b(b_), a(a_) {}
        explicit constexpr ColorRGBA(uint32_t packedValue_) : packedValue(packedValue_) {}
        explicit ColorRGBA(_In_reads_(4) const uint8_t *pArray) : r(pArray[0]), g(pArray[1]), b(pArray[2]), a(pArray[3]) {}
        //ColorRGBA(float _x, float _y, float _z, float _w);
        //explicit ColorRGBA(_In_reads_(4) const float *pArray);

        ColorRGBA& operator= (uint32_t packedValue_) { packedValue  = packedValue_; return *this; }
    };

	/// Defines 4-element floating point RGBA color.
	class ALIMER_API Color4
	{
	public:
		/// Red value.
		float r;

		/// Green value.
		float g;

		/// Blue value.
		float b;

		/// Alpha value.
		float a;

		/// Construct with default values (opaque black.)
        Color4() noexcept : r(0.0f), g(0.0f), b(0.0f), a(1.0f)
		{
		}

		constexpr Color4(float r_, float g_, float b_) : r(r_), g(g_), b(b_), a(1.0f) {}
		constexpr Color4(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) {}

        explicit Color4(const vec3& vector) : r(vector.x), g(vector.y), b(vector.z), a(1.0f) {}
        explicit Color4(const vec4& vector) : r(vector.x), g(vector.y), b(vector.z), a(vector.w) {}

        Color4(const Color4&) = default;
        Color4& operator=(const Color4&) = default;

        Color4(Color4&&) = default;
        Color4& operator=(Color4&&) = default;

		/// Operators
		operator const float*() const { return reinterpret_cast<const float*>(this); }

		/// Test for equality with another color without epsilon.
		bool operator ==(const Color4& rhs) const { return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a; }

		/// Test for inequality with another color without epsilon.
		bool operator !=(const Color4& rhs) const { return r != rhs.r || g != rhs.g || b != rhs.b || a != rhs.a; }

		/// Return as string.
		String ToString() const;

        /// Return float data.
        const float* Data() const { return &r; }

		/// Black color.
		static const Color4 Black;
		static const Color4 White;
        static const Color4 WhiteSmoke;
        static const Color4 Red;
        static const Color4 Green;
        static const Color4 Blue;
        static const Color4 Yellow;
        static const Color4 YellowGreen;
        static const Color4 Lime;
	};

    //inline ColorBGRA::ColorBGRA(float _r, float _g, float _b, float _a)
    //{
        // TODO:
    //}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
