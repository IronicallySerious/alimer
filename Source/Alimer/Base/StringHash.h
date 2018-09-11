//
// Alimer is based on the Turso3D codebase.
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

#include "../Base/String.h"
#include "../Math/MathUtil.h"

namespace Alimer
{
	/// 32-bit case-insensitive hash value for a string.
	class ALIMER_API StringHash
	{
	public:
		/// Construct with zero value.
		StringHash() noexcept : _value(0) {}

		/// Copy-construct.
		StringHash(const StringHash& rhs) noexcept = default;

		/// Construct with an initial value.
		explicit StringHash(uint32_t value) noexcept : _value(value) { }

		/// Construct from a C string case-insensitively.
		StringHash(const char* str) noexcept // NOLINT(google-explicit-constructor)
            : _value(Calculate(str))
        {
        }

		/// Construct from a string case-insensitively.
		StringHash(const String& str) noexcept;      // NOLINT(google-explicit-constructor)

        /// Add a hash.
        StringHash operator +(const StringHash& rhs) const
        {
            StringHash ret;
            ret._value = _value + rhs._value;
            return ret;
        }

        /// Add-assign a hash.
        StringHash& operator +=(const StringHash& rhs)
        {
            _value += rhs._value;
            return *this;
        }

		/// Test for equality with another hash.
		bool operator == (const StringHash& rhs) const { return _value == rhs._value; }
		/// Test for inequality with another hash.
		bool operator != (const StringHash& rhs) const { return _value != rhs._value; }
		/// Test if less than another hash.
		bool operator < (const StringHash& rhs) const { return _value < rhs._value; }
		/// Test if greater than another hash.
		bool operator > (const StringHash& rhs) const { return _value > rhs._value; }
		/// Return true if nonzero hash value.
		operator bool() const { return _value != 0; }
		/// Return hash value.
		uint32_t Value() const { return _value; }
		/// Return as string.
		String ToString() const;

		/// Return hash value for HashSet & HashMap.
		uint32_t ToHash() const { return _value; }

        /// Calculate hash value case-insensitively from a C string.
        static constexpr uint32_t Calculate(const char* str, uint32_t hash = 0)
        {
            return str == nullptr || *str == 0 ? hash : Calculate(str + 1, SDBMHash(hash, (unsigned char)(((*str) >= 'A' && (*str) <= 'Z') ? (*str) + ('a' - 'A') : (*str))));
        }

        /// Calculate hash value from binary data.
        static uint32_t Calculate(void* data, uint32_t length, uint32_t hash = 0);

		/// Zero hash.
		static const StringHash ZERO;

	private:
		/// Hash value.
		uint32_t _value;
	};
}

namespace std {
	template<>
	class hash<Alimer::StringHash> {
	public:
		size_t operator()(const Alimer::StringHash &s) const
		{
			return s.ToHash();
		}
	};
}
