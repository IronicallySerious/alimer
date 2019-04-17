//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "String.h"
#include "WString.h"

#include <cstring>

#include "../Debug/DebugNew.h"

namespace Turso3D
{

    WString::WString() :
        length(0),
        buffer(nullptr)
    {
    }

    WString::WString(const String& str) :
        length(0),
        buffer(nullptr)
    {
#ifdef _WIN32
        size_t neededSize = 0;
        wchar_t temp[3];

        size_t byteOffset = 0;
        while (byteOffset < str.Length())
        {
            wchar_t* dest = temp;
            String::EncodeUTF16(dest, str.NextUTF8Char(byteOffset));
            neededSize += dest - temp;
        }

        Resize(neededSize);

        byteOffset = 0;
        wchar_t* dest = buffer;
        while (byteOffset < str.Length())
            String::EncodeUTF16(dest, str.NextUTF8Char(byteOffset));
#else
        Resize(str.LengthUTF8());

        size_t byteOffset = 0;
        wchar_t* dest = buffer;
        while (byteOffset < str.Length())
            *dest++ = str.NextUTF8Char(byteOffset);
#endif
}

    WString::~WString()
    {
        delete[] buffer;
    }

    void WString::Resize(size_t newLength)
    {
        if (!newLength)
        {
            delete[] buffer;
            buffer = nullptr;
            length = 0;
        }
        else
        {
            wchar_t* newBuffer = new wchar_t[newLength + 1];
            if (buffer)
            {
                size_t copyLength = length < newLength ? length : newLength;
                memcpy(newBuffer, buffer, copyLength * sizeof(wchar_t));
                delete[] buffer;
            }
            newBuffer[newLength] = 0;
            buffer = newBuffer;
            length = newLength;
        }
    }

}
