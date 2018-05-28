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
	class Matrix4x4;

	/**
	* Defines 4-element floating point vector.
	*/
	class Vector4
	{
	public:
		/**
		* The x-coordinate.
		*/
		float x;

		/**
		* The y-coordinate.
		*/
		float y;

		/**
		* The z-coordinate.
		*/
		float z;

		/**
		* The w-coordinate.
		*/
		float w;

		/// Construct with all values set to zero.
		Vector4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }

		constexpr explicit Vector4(float value) : x(value), y(value), z(value), w(value) { }

		constexpr Vector4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
		explicit Vector4(_In_reads_(4) const float *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3]) {}

		Vector4(const Vector4&) = default;
		Vector4& operator=(const Vector4&) = default;

		Vector4(Vector4&&) = default;
		Vector4& operator=(Vector4&&) = default;

		/// Test for equality with another color without epsilon.
		inline bool operator ==(const Vector4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }

		/// Test for inequality with another color without epsilon.
		inline bool operator !=(const Vector4& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w; }

		/// Return as string.
		std::string ToString() const;

		/// Sets the x, y, z and w to the specified values.
		void Set(float x_, float y_, float z_, float w_);

		static const Vector4 Zero;
		static const Vector4 One;
		static const Vector4 UnitX;
		static const Vector4 UnitY;
		static const Vector4 UnitZ;
		static const Vector4 UnitW;
	};
}
