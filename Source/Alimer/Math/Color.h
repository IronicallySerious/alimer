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

#include "../PlatformDef.h"
#include <string>

namespace Alimer
{

	/**
	* Defines 4-element floating point RGBA color.
	*/
	class Color
	{
	public:

		/**
		* Red value.
		*/
		float r;

		/**
		* Green value.
		*/
		float g;

		/**
		* Blue value.
		*/
		float b;

		/**
		* Alpha value..
		*/
		float a;

		/// Construct with default values (opaque black.)
		Color() noexcept : r(0.0f), g(0.0f), b(0.0f), a(0.0f)
		{
		}

		constexpr Color(float r_, float g_, float b_)
			: r(r_), g(g_), b(b_), a(1.0f) {}
		constexpr Color(float r_, float g_, float b_, float a_)
			: r(r_), g(g_), b(b_), a(a_) {}

		Color(const Color&) = default;
		Color& operator=(const Color&) = default;

		Color(Color&&) = default;
		Color& operator=(Color&&) = default;

		/// Operators
		operator const float*() const { return reinterpret_cast<const float*>(this); }

		/// Test for equality with another color without epsilon.
		bool operator ==(const Color& rhs) const { return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a; }

		/// Test for inequality with another color without epsilon.
		bool operator !=(const Color& rhs) const { return r != rhs.r || g != rhs.g || b != rhs.b || a != rhs.a; }

		/// Return as string.
		std::string ToString() const;

		/// Black color.
		static const Color Black;

		/// White color.
		static const Color White;
	};
}
