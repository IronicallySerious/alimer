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

#include "../Math/Vector3.h"
#include "../Math/Vector4.h"

namespace Alimer
{
	class Matrix4x4;

	/**
	* Defines 4-element floating point quaternion.
	*/
	class Quaternion
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

		/// Construct an identity quaternion
		Quaternion() noexcept : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
		{
		}

		constexpr Quaternion(float x_, float y_, float z_, float w_)
			: x(x_), y(y_), z(z_), w(w_) {}

		Quaternion(const Vector3& v, float scalar) : x(v.x), y(v.y), z(v.z), w(scalar) {}
		explicit Quaternion(const Vector4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
		explicit Quaternion(_In_reads_(4) const float *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3]) {}

		Quaternion(const Quaternion&) = default;
		Quaternion& operator=(const Quaternion&) = default;

		Quaternion(Quaternion&&) = default;
		Quaternion& operator=(Quaternion&&) = default;

		/// Test for equality with another color without epsilon.
		bool operator ==(const Quaternion& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }

		/// Test for inequality with another color without epsilon.
		bool operator !=(const Quaternion& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w; }

		/// Return as string.
		std::string ToString() const;

		/// Sets the x, y, z and w to the specified values.
		void Set(float x_, float y_, float z_, float w_);

		static const Quaternion Identity;
	};
}
