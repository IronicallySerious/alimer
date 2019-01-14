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

#include "../Base/StringHash.h"
#include "../Base/String.h"

namespace alimer
{
	const StringHash StringHash::ZERO;

    StringHash::StringHash(const std::string& str) noexcept
        : _value(Calculate(str.c_str()))
    {
    }

	StringHash::StringHash(const String& str) noexcept
		: _value(Calculate(str.CString()))
	{
	}

	String StringHash::ToString() const
	{
		char tempBuffer[CONVERSION_BUFFER_LENGTH];
		sprintf(tempBuffer, "%08X", _value);
		return String(tempBuffer);
	}

    uint32_t StringHash::Calculate(void* data, uint32_t length, uint32_t hash)
    {
        if (!data)
            return hash;

        auto* bytes = static_cast<uint8_t*>(data);
        auto* end = bytes + length;
        while (bytes < end)
        {
            hash = SDBMHash(hash, *bytes);
            ++bytes;
        }

        return hash;
    }
}
