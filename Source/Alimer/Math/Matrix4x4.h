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
	/**
	* Defines a 4x4 floating point right-handed matrix.
	*/
	class Matrix4x4
	{
	public:
		union
		{
			struct
			{
				float m11, m12, m13, m14;
				float m21, m22, m23, m24;
				float m31, m32, m33, m34;
				float m41, m42, m43, m44;
			};
			float m[4][4];
		};

		/// Construct an identity matrix
		Matrix4x4() noexcept :
			m11(1.0f), m12(0.0f), m13(0.0f), m14(0.0f),
			m21(0.0f), m22(1.0f), m23(0.0f), m24(0.0f),
			m31(0.0f), m32(0.0f), m33(1.0f), m34(0.0f),
			m41(0.0f), m42(0.0f), m43(0.0f), m44(1.0f)
		{
		}

		constexpr Matrix4x4(
			float m11_, float m12_, float m13_, float m14_,
			float m21_, float m22_, float m23_, float m24_,
			float m31_, float m32_, float m33_, float m34_,
			float m41_, float m42_, float m43_, float m44_) :
			m11(m11_), m12(m12_), m13(m13_), m14(m14_),
			m21(m21_), m22(m22_), m23(m23_), m24(m24_),
			m31(m31_), m32(m32_), m33(m33_), m34(m34_),
			m41(m41_), m42(m42_), m43(m43_), m44(m44_)
		{
		}

		explicit Matrix4x4(_In_reads_(16) const float *pArray);

		Matrix4x4(const Matrix4x4&) = default;
		Matrix4x4& operator=(const Matrix4x4&) = default;

		Matrix4x4(Matrix4x4&&) = default;
		Matrix4x4& operator=(Matrix4x4&&) = default;

		/// Test for equality with another color without epsilon.
		bool operator ==(const Matrix4x4& rhs) const
		{
			// Diagonal first.
			return
				m11 == rhs.m11 && m22 == rhs.m22 && m33 == rhs.m33 && m44 == rhs.m44
				&& m12 == rhs.m12 && m13 == rhs.m13 && m14 == rhs.m14
				&& m21 == rhs.m21 && m23 == rhs.m23 && m24 == rhs.m24
				&& m31 == rhs.m31 && m32 == rhs.m32 && m34 == rhs.m34
				&& m41 == rhs.m41 && m42 == rhs.m42 && m43 == rhs.m43;
		}

		/// Test for inequality with another color without epsilon.
		bool operator !=(const Matrix4x4& rhs) const { return !(*this == rhs); }

		float       operator() (size_t row, size_t column) const { return m[row][column]; }
		float&      operator() (size_t row, size_t column) { return m[row][column]; }

		/// Return as string.
		std::string ToString() const;

		static const Matrix4x4 Identity;
	};
}
