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

#pragma once

#include "Turso3DConfig.h"
#include <cassert>
#include <cstddef>

namespace Turso3D
{

    class String;

    /// Wide character string. Only meant for converting from String and passing to the operating system where necessary.
    class TURSO3D_API WString
    {
    public:
        /// Construct empty.
        WString();
        /// Construct from a string.
        WString(const String& str);
        /// Destruct.
        ~WString();

        /// Return char at index.
        wchar_t& operator [] (size_t index) { assert(index < length); return buffer[index]; }
        /// Return const char at index.
        const wchar_t& operator [] (size_t index) const { assert(index < length); return buffer[index]; }
        /// Return char at index.
        wchar_t& At(size_t index) { assert(index < length); return buffer[index]; }
        /// Return const char at index.
        const wchar_t& At(size_t index) const { assert(index < length); return buffer[index]; }
        /// Resize the string.
        void Resize(size_t newLength);
        /// Return whether the string is zero characters long.
        bool IsEmpty() const { return length == 0; }
        /// Return number of characters.
        size_t Length() const { return length; }
        /// Return character data.
        const wchar_t* CString() const { return buffer; }

    private:
        /// String length.
        size_t length;
        /// String buffer, null if not allocated.
        wchar_t* buffer;
    };

}
