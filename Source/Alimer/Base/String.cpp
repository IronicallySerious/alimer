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

#include "../Base/String.h"
#include "../Base/StringHash.h"
#include "../Debug/Log.h"

namespace Alimer
{
    char String::END_ZERO = 0;
    const String String::EMPTY;

    String::String(const WString& str)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        SetUTF8FromWChar(str.CString());
    }

    String::String(int value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%d", value);
        *this = tempBuffer;
    }

    String::String(short value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%d", value);
        *this = tempBuffer;
    }

    String::String(long value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%ld", value);
        *this = tempBuffer;
    }

    String::String(long long value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%lld", value);
        *this = tempBuffer;
    }

    String::String(unsigned value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%u", value);
        *this = tempBuffer;
    }

    String::String(unsigned short value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%u", value);
        *this = tempBuffer;
    }

    String::String(unsigned long value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%lu", value);
        *this = tempBuffer;
    }

    String::String(unsigned long long value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%llu", value);
        *this = tempBuffer;
    }

    String::String(float value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%g", value);
        *this = tempBuffer;
    }

    String::String(double value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%.15g", value);
        *this = tempBuffer;
    }

    String::String(bool value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        if (value)
            *this = "true";
        else
            *this = "false";
    }

    String::String(char value)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        Resize(1);
        _buffer[0] = value;
    }

    String::String(char value, uint32_t length)
        : _length(0)
        , _capacity(0)
        , _buffer(&END_ZERO)
    {
        Resize(length);
        for (uint32_t i = 0; i < length; ++i)
            _buffer[i] = value;
    }

    String& String::operator +=(int rhs)
    {
        return *this += String(rhs);
    }

    String& String::operator +=(short rhs)
    {
        return *this += String(rhs);
    }

    String& String::operator +=(long rhs)
    {
        return *this += String(rhs);
    }

    String& String::operator +=(long long rhs)
    {
        return *this += String(rhs);
    }

    String& String::operator +=(unsigned rhs)
    {
        return *this += String(rhs);
    }

    String& String::operator +=(unsigned short rhs)
    {
        return *this += String(rhs);
    }

    String& String::operator +=(unsigned long rhs)
    {
        return *this += String(rhs);
    }

    String& String::operator +=(unsigned long long rhs)
    {
        return *this += String(rhs);
    }

    String& String::operator +=(float rhs)
    {
        return *this += String(rhs);
    }

    String& String::operator +=(bool rhs)
    {
        return *this += String(rhs);
    }

    void String::Resize(uint32_t newLength)
    {
        if (!_capacity)
        {
            // If zero length requested, do not allocate buffer yet
            if (!newLength)
                return;

            // Calculate initial capacity
            _capacity = newLength + 1;
            if (_capacity < MIN_CAPACITY)
                _capacity = MIN_CAPACITY;

            _buffer = new char[_capacity];
        }
        else
        {
            if (newLength && _capacity < newLength + 1)
            {
                // Increase the capacity with half each time it is exceeded
                while (_capacity < newLength + 1)
                {
                    _capacity += (_capacity + 1) >> 1u;
                }

                auto* newBuffer = new char[_capacity];
                // Move the existing data to the new buffer, then delete the old buffer
                if (_length)
                {
                    CopyChars(newBuffer, _buffer, _length);
                }
                delete[] _buffer;

                _buffer = newBuffer;
            }
        }

        _buffer[newLength] = 0;
        _length = newLength;
    }

    void String::Reserve(uint32_t newCapacity)
    {
        if (newCapacity < _length + 1)
            newCapacity = _length + 1;
        if (newCapacity == _capacity)
            return;

        auto* newBuffer = new char[newCapacity];
        // Move the existing data to the new buffer, then delete the old buffer
        CopyChars(newBuffer, _buffer, _length + 1);
        if (_capacity)
        {
            delete[] _buffer;
        }

        _capacity = newCapacity;
        _buffer = newBuffer;
    }

    void String::Compact()
    {
        if (_capacity)
        {
            Reserve(_length + 1);
        }
    }

    void String::Clear()
    {
        Resize(0);
    }

    void String::Swap(String& str)
    {
        Alimer::Swap(_length, str._length);
        Alimer::Swap(_capacity, str._capacity);
        Alimer::Swap(_buffer, str._buffer);
    }

    int String::Compare(const String& str, bool caseSensitive) const
    {
        return Compare(CString(), str.CString(), caseSensitive);
    }

    int String::Compare(const char* str, bool caseSensitive) const
    {
        return Compare(CString(), str, caseSensitive);
    }

    int String::Compare(const char* lhs, const char* rhs, bool caseSensitive)
    {
        if (!lhs || !rhs)
            return lhs ? 1 : (rhs ? -1 : 0);

        if (caseSensitive)
            return strcmp(lhs, rhs);
        else
        {
            for (;;)
            {
                auto l = (char)tolower(*lhs);
                auto r = (char)tolower(*rhs);
                if (!l || !r)
                    return l ? 1 : (r ? -1 : 0);
                if (l < r)
                    return -1;
                if (l > r)
                    return 1;

                ++lhs;
                ++rhs;
            }
        }
    }

    void String::Replace(char replaceThis, char replaceWith, bool caseSensitive)
    {
        if (caseSensitive)
        {
            for (uint32_t i = 0; i < _length; ++i)
            {
                if (_buffer[i] == replaceThis)
                    _buffer[i] = replaceWith;
            }
        }
        else
        {
            replaceThis = (char)tolower(replaceThis);
            for (uint32_t i = 0; i < _length; ++i)
            {
                if (tolower(_buffer[i]) == replaceThis)
                    _buffer[i] = replaceWith;
            }
        }
    }

    void String::Replace(const String& replaceThis, const String& replaceWith, bool caseSensitive)
    {
        uint32_t nextPos = 0;

        while (nextPos < _length)
        {
            uint32_t pos = Find(replaceThis, nextPos, caseSensitive);
            if (pos == NPOS)
                break;
            Replace(pos, replaceThis._length, replaceWith);
            nextPos = pos + replaceWith._length;
        }
    }

    void String::Replace(uint32_t pos, uint32_t length, const String& replaceWith)
    {
        // If substring is illegal, do nothing
        if (pos + length > _length)
            return;

        Replace(pos, length, replaceWith._buffer, replaceWith._length);
    }

    void String::Replace(uint32_t pos, uint32_t length, const char* replaceWith)
    {
        // If substring is illegal, do nothing
        if (pos + length > _length)
            return;

        Replace(pos, length, replaceWith, CStringLength(replaceWith));
    }

    String::Iterator String::Replace(const String::Iterator& start, const String::Iterator& end, const String& replaceWith)
    {
        uint32_t pos = (uint32_t)(start - Begin());
        if (pos >= _length)
            return End();
        auto length = (uint32_t)(end - start);
        Replace(pos, length, replaceWith);

        return Begin() + pos;
    }

    String String::Replaced(char replaceThis, char replaceWith, bool caseSensitive) const
    {
        String ret(*this);
        ret.Replace(replaceThis, replaceWith, caseSensitive);
        return ret;
    }

    String String::Replaced(const String& replaceThis, const String& replaceWith, bool caseSensitive) const
    {
        String ret(*this);
        ret.Replace(replaceThis, replaceWith, caseSensitive);
        return ret;
    }


    String& String::Append(const String& str)
    {
        return *this += str;
    }

    String& String::Append(const char* str)
    {
        return *this += str;
    }

    String& String::Append(char c)
    {
        return *this += c;
    }

    String& String::Append(const char* str, uint32_t length)
    {
        if (str)
        {
            uint32_t oldLength = _length;
            Resize(oldLength + length);
            CopyChars(&_buffer[oldLength], str, length);
        }

        return *this;
    }

    String& String::AppendWithFormat(const char* formatString, ...)
    {
        va_list args;
        va_start(args, formatString);
        AppendWithFormatArgs(formatString, args);
        va_end(args);
        return *this;
    }

    String& String::AppendWithFormatArgs(const char* formatString, va_list args)
    {
        int pos = 0, lastPos = 0;
        auto length = (int)strlen(formatString);

        while (true)
        {
            // Scan the format string and find %a argument where a is one of d, f, s ...
            while (pos < length && formatString[pos] != '%') pos++;
            Append(formatString + lastPos, (unsigned)(pos - lastPos));
            if (pos >= length)
                return *this;

            char format = formatString[pos + 1];
            pos += 2;
            lastPos = pos;

            switch (format)
            {
                // Integer
            case 'd':
            case 'i':
            {
                int arg = va_arg(args, int);
                Append(String(arg));
                break;
            }

            // Unsigned
            case 'u':
            {
                unsigned arg = va_arg(args, unsigned);
                Append(String(arg));
                break;
            }

            // Unsigned long
            case 'l':
            {
                unsigned long arg = va_arg(args, unsigned long);
                Append(String(arg));
                break;
            }

            // Real
            case 'f':
            {
                double arg = va_arg(args, double);
                Append(String(arg));
                break;
            }

            // Character
            case 'c':
            {
                int arg = va_arg(args, int);
                Append((char)arg);
                break;
            }

            // C string
            case 's':
            {
                char* arg = va_arg(args, char*);
                Append(arg);
                break;
            }

            // Hex
            case 'x':
            {
                char buf[CONVERSION_BUFFER_LENGTH];
                int arg = va_arg(args, int);
                int arglen = ::sprintf(buf, "%x", arg);
                Append(buf, (unsigned)arglen);
                break;
            }

            // Pointer
            case 'p':
            {
                char buf[CONVERSION_BUFFER_LENGTH];
                int arg = va_arg(args, int);
                int arglen = ::sprintf(buf, "%p", reinterpret_cast<void*>((uintptr_t)arg));
                Append(buf, (unsigned)arglen);
                break;
            }

            case '%':
            {
                Append("%", 1);
                break;
            }

            default:
                ALIMER_LOGWARNF("String", "Unsupported format specifier: '%c'", format);
                break;
            }
        }
    }

    void String::Insert(uint32_t pos, const String& str)
    {
        if (pos > _length)
            pos = _length;

        if (pos == _length)
            (*this) += str;
        else
            Replace(pos, 0, str);
    }

    void String::Insert(uint32_t pos, char c)
    {
        if (pos > _length)
            pos = _length;

        if (pos == _length)
        {
            (*this) += c;
        }
        else
        {
            uint32_t oldLength = _length;
            Resize(_length + 1);
            MoveRange(pos + 1, pos, oldLength - pos);
            _buffer[pos] = c;
        }
    }

    String::Iterator String::Insert(const String::Iterator& dest, const String& str)
    {
        auto pos = (uint32_t)(dest - Begin());
        if (pos > _length)
            pos = _length;
        Insert(pos, str);

        return Begin() + pos;
    }

    String::Iterator String::Insert(const String::Iterator& dest, const String::Iterator& start, const String::Iterator& end)
    {
        auto pos = (uint32_t)(dest - Begin());
        if (pos > _length)
            pos = _length;
        auto length = (uint32_t)(end - start);
        Replace(pos, 0, &(*start), length);

        return Begin() + pos;
    }

    String::Iterator String::Insert(const String::Iterator& dest, char c)
    {
        auto pos = (uint32_t)(dest - Begin());
        if (pos > _length)
            pos = _length;
        Insert(pos, c);

        return Begin() + pos;
    }

    void String::Erase(uint32_t pos, uint32_t length)
    {
        Replace(pos, length, String::EMPTY);
    }

    String::Iterator String::Erase(const String::Iterator& it)
    {
        auto pos = (uint32_t)(it - Begin());
        if (pos >= _length)
            return End();
        Erase(pos);

        return Begin() + pos;
    }

    String::Iterator String::Erase(const String::Iterator& start, const String::Iterator& end)
    {
        auto pos = (uint32_t)(start - Begin());
        if (pos >= _length)
            return End();
        auto length = (uint32_t)(end - start);
        Erase(pos, length);

        return Begin() + pos;
    }

    String String::Substring(uint32_t pos) const
    {
        if (pos < _length)
        {
            String ret;
            ret.Resize(_length - pos);
            CopyChars(ret._buffer, _buffer + pos, ret._length);

            return ret;
        }

        return String();
    }

    String String::Substring(uint32_t pos, uint32_t length) const
    {
        if (pos < _length)
        {
            String ret;
            if (pos + length > _length)
                length = _length - pos;
            ret.Resize(length);
            CopyChars(ret._buffer, _buffer + pos, ret._length);

            return ret;
        }
        else
            return String();
    }

    String String::Trimmed(const String& chars) const
    {
        uint32_t trimStart = 0;
        uint32_t trimEnd = _length;

        while (trimStart < trimEnd)
        {
            char c = _buffer[trimStart];
            if (!chars.Contains(c))
                break;
            ++trimStart;
        }
        while (trimEnd > trimStart)
        {
            char c = _buffer[trimEnd - 1];
            if (!chars.Contains(c))
                break;
            --trimEnd;
        }

        return Substring(trimStart, trimEnd - trimStart);
    }

    String String::ToLower() const
    {
        String ret(*this);
        for (uint32_t i = 0; i < ret._length; ++i)
            ret[i] = (char)tolower(_buffer[i]);

        return ret;
    }

    String String::ToUpper() const
    {
        String ret(*this);
        for (uint32_t i = 0; i < ret._length; ++i)
            ret[i] = (char)toupper(_buffer[i]);

        return ret;
    }

    Vector<String> String::Split(char separator, bool keepEmptyStrings) const
    {
        return Split(CString(), separator, keepEmptyStrings);
    }

    void String::Join(const Vector<String>& subStrings, const String& glue)
    {
        *this = Joined(subStrings, glue);
    }

    uint32_t String::Find(const String& str, uint32_t startPos, bool caseSensitive) const
    {
        if (!str._length || str._length > _length)
            return NPOS;

        char first = str._buffer[0];
        if (!caseSensitive)
            first = (char)tolower(first);

        for (uint32_t i = startPos; i <= _length - str._length; ++i)
        {
            char c = _buffer[i];
            if (!caseSensitive)
                c = (char)tolower(c);

            if (c == first)
            {
                uint32_t skip = NPOS;
                bool found = true;
                for (uint32_t j = 1; j < str._length; ++j)
                {
                    c = _buffer[i + j];
                    char d = str._buffer[j];
                    if (!caseSensitive)
                    {
                        c = (char)tolower(c);
                        d = (char)tolower(d);
                    }

                    if (skip == NPOS && c == first)
                        skip = i + j - 1;

                    if (c != d)
                    {
                        found = false;
                        if (skip != NPOS)
                            i = skip;
                        break;
                    }
                }
                if (found)
                    return i;
            }
        }

        return NPOS;
    }

    uint32_t String::Find(char c, uint32_t startPos, bool caseSensitive) const
    {
        if (caseSensitive)
        {
            for (uint32_t i = startPos; i < _length; ++i)
            {
                if (_buffer[i] == c)
                    return i;
            }
        }
        else
        {
            c = (char)tolower(c);
            for (uint32_t i = startPos; i < _length; ++i)
            {
                if (tolower(_buffer[i]) == c)
                    return i;
            }
        }

        return NPOS;
    }

    uint32_t String::FindLast(const String& str, uint32_t startPos, bool caseSensitive) const
    {
        if (!str._length || str._length > _length)
            return NPOS;

        if (startPos > _length - str._length)
            startPos = _length - str._length;

        char first = str._buffer[0];
        if (!caseSensitive)
            first = (char)tolower(first);

        for (uint32_t i = startPos; i < _length; --i)
        {
            char c = _buffer[i];
            if (!caseSensitive)
                c = (char)tolower(c);

            if (c == first)
            {
                bool found = true;
                for (uint32_t j = 1; j < str._length; ++j)
                {
                    c = _buffer[i + j];
                    char d = str._buffer[j];
                    if (!caseSensitive)
                    {
                        c = (char)tolower(c);
                        d = (char)tolower(d);
                    }

                    if (c != d)
                    {
                        found = false;
                        break;
                    }
                }
                if (found)
                    return i;
            }
        }

        return NPOS;
    }

    uint32_t String::FindLast(char c, uint32_t startPos, bool caseSensitive) const
    {
        if (startPos >= _length)
            startPos = _length - 1;

        if (caseSensitive)
        {
            for (uint32_t i = startPos; i < _length; --i)
            {
                if (_buffer[i] == c)
                    return i;
            }
        }
        else
        {
            c = (char)tolower(c);
            for (uint32_t i = startPos; i < _length; --i)
            {
                if (tolower(_buffer[i]) == c)
                    return i;
            }
        }

        return NPOS;
    }

    bool String::StartsWith(const String& str, bool caseSensitive) const
    {
        return Find(str, 0, caseSensitive) == 0;
    }

    bool String::EndsWith(const String& str, bool caseSensitive) const
    {
        uint32_t pos = FindLast(str, Length() - 1, caseSensitive);
        return pos != NPOS && pos == Length() - str.Length();
    }

    Vector<String> String::Split(const char* str, char separator, bool keepEmptyStrings)
    {
        Vector<String> ret;
        const char* strEnd = str + String::CStringLength(str);

        for (const char* splitEnd = str; splitEnd != strEnd; ++splitEnd)
        {
            if (*splitEnd == separator)
            {
                const ptrdiff_t splitLen = splitEnd - str;
                if (splitLen > 0 || keepEmptyStrings)
                {
                    ret.Push(String(str, static_cast<uint32_t>(splitLen)));
                }

                str = splitEnd + 1;
            }
        }

        const ptrdiff_t splitLen = strEnd - str;
        if (splitLen > 0 || keepEmptyStrings)
        {
            ret.Push(String(str, static_cast<uint32_t>(splitLen)));
        }

        return ret;
    }

    String String::Joined(const Vector<String>& subStrings, const String& glue)
    {
        if (subStrings.IsEmpty())
            return String();

        String joinedString(subStrings[0]);
        for (uint32_t i = 1; i < subStrings.Size(); ++i)
        {
            joinedString.Append(glue).Append(subStrings[i]);
        }

        return joinedString;
    }

    String String::Format(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        char buf[256];
        uint32_t n = std::vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);

        // Static buffer large enough?
        if (n < sizeof(buf))
        {
            return String(buf, n);
        }

        // Static buffer too small
        String s;
        s.Resize(n + 1);
        va_start(args, format);
        std::vsnprintf(s._buffer, s.Length(), format, args);
        va_end(args);
        return s;
    }

    void String::Replace(uint32_t pos, uint32_t length, const char* srcStart, uint32_t srcLength)
    {
        int delta = (int)srcLength - (int)length;

        if (pos + length < _length)
        {
            if (delta < 0)
            {
                MoveRange(pos + srcLength, pos + length, _length - pos - length);
                Resize(_length + delta);
            }
            if (delta > 0)
            {
                Resize(_length + delta);
                MoveRange(pos + srcLength, pos + length, _length - pos - length - delta);
            }
        }
        else
        {
            Resize(_length + delta);
        }

        CopyChars(_buffer + pos, srcStart, srcLength);
    }

    void String::SetUTF8FromLatin1(const char* str)
    {
        char temp[7];

        Clear();

        if (!str)
            return;

        while (*str)
        {
            char* dest = temp;
            EncodeUTF8(dest, (uint32_t)*str++);
            *dest = 0;
            Append(temp);
        }
    }

    void String::SetUTF8FromWChar(const wchar_t* str)
    {
        char temp[7];

        Clear();

        if (!str)
            return;

#ifdef _WIN32
        while (*str)
        {
            uint32_t unicodeChar = DecodeUTF16(str);
            char* dest = temp;
            EncodeUTF8(dest, unicodeChar);
            *dest = 0;
            Append(temp);
        }
#else
        while (*str)
        {
            char* dest = temp;
            EncodeUTF8(dest, (uint32_t)*str++);
            *dest = 0;
            Append(temp);
        }
#endif
    }

    uint32_t String::LengthUTF8() const
    {
        uint32_t ret = 0;

        const char* src = _buffer;
        if (!src)
            return ret;
        const char* end = _buffer + _length;

        while (src < end)
        {
            DecodeUTF8(src);
            ++ret;
        }

        return ret;
    }

    uint32_t String::ByteOffsetUTF8(uint32_t index) const
    {
        uint32_t byteOffset = 0;
        uint32_t utfPos = 0;

        while (utfPos < index && byteOffset < _length)
        {
            NextUTF8Char(byteOffset);
            ++utfPos;
        }

        return byteOffset;
    }

    uint32_t String::NextUTF8Char(uint32_t& byteOffset) const
    {
        if (!_buffer)
            return 0;

        const char* src = _buffer + byteOffset;
        uint32_t ret = DecodeUTF8(src);
        byteOffset = (uint32_t)(src - _buffer);

        return ret;
    }

    uint32_t String::AtUTF8(uint32_t index) const
    {
        uint32_t byteOffset = ByteOffsetUTF8(index);
        return NextUTF8Char(byteOffset);
    }

    void String::ReplaceUTF8(uint32_t index, uint32_t unicodeChar)
    {
        uint32_t utfPos = 0;
        uint32_t byteOffset = 0;

        while (utfPos < index && byteOffset < _length)
        {
            NextUTF8Char(byteOffset);
            ++utfPos;
        }

        if (utfPos < index)
            return;

        uint32_t beginCharPos = byteOffset;
        NextUTF8Char(byteOffset);

        char temp[7];
        char* dest = temp;
        EncodeUTF8(dest, unicodeChar);
        *dest = 0;

        Replace(beginCharPos, byteOffset - beginCharPos, temp, (uint32_t)(dest - temp));
    }

    String& String::AppendUTF8(uint32_t unicodeChar)
    {
        char temp[7];
        char* dest = temp;
        EncodeUTF8(dest, unicodeChar);
        *dest = 0;
        return Append(temp);
    }

    String String::SubstringUTF8(uint32_t pos) const
    {
        uint32_t utf8Length = LengthUTF8();
        uint32_t byteOffset = ByteOffsetUTF8(pos);
        String ret;

        while (pos < utf8Length)
        {
            ret.AppendUTF8(NextUTF8Char(byteOffset));
            ++pos;
        }

        return ret;
    }

    String String::SubstringUTF8(uint32_t pos, uint32_t length) const
    {
        uint32_t utf8Length = LengthUTF8();
        uint32_t byteOffset = ByteOffsetUTF8(pos);
        uint32_t endPos = pos + length;
        String ret;

        while (pos < endPos && pos < utf8Length)
        {
            ret.AppendUTF8(NextUTF8Char(byteOffset));
            ++pos;
        }

        return ret;
    }

    void String::EncodeUTF8(char*& dest, uint32_t unicodeChar)
    {
        if (unicodeChar < 0x80)
        {
            *dest++ = (char)unicodeChar;
        }
        else if (unicodeChar < 0x800)
        {
            dest[0] = (char)(0xc0u | ((unicodeChar >> 6u) & 0x1fu));
            dest[1] = (char)(0x80u | (unicodeChar & 0x3fu));
            dest += 2;
        }
        else if (unicodeChar < 0x10000)
        {
            dest[0] = (char)(0xe0u | ((unicodeChar >> 12u) & 0xfu));
            dest[1] = (char)(0x80u | ((unicodeChar >> 6u) & 0x3fu));
            dest[2] = (char)(0x80u | (unicodeChar & 0x3fu));
            dest += 3;
        }
        else if (unicodeChar < 0x200000)
        {
            dest[0] = (char)(0xf0u | ((unicodeChar >> 18u) & 0x7u));
            dest[1] = (char)(0x80u | ((unicodeChar >> 12u) & 0x3fu));
            dest[2] = (char)(0x80u | ((unicodeChar >> 6u) & 0x3fu));
            dest[3] = (char)(0x80u | (unicodeChar & 0x3fu));
            dest += 4;
        }
        else if (unicodeChar < 0x4000000)
        {
            dest[0] = (char)(0xf8u | ((unicodeChar >> 24u) & 0x3u));
            dest[1] = (char)(0x80u | ((unicodeChar >> 18u) & 0x3fu));
            dest[2] = (char)(0x80u | ((unicodeChar >> 12u) & 0x3fu));
            dest[3] = (char)(0x80u | ((unicodeChar >> 6u) & 0x3fu));
            dest[4] = (char)(0x80u | (unicodeChar & 0x3fu));
            dest += 5;
        }
        else
        {
            dest[0] = (char)(0xfcu | ((unicodeChar >> 30u) & 0x1u));
            dest[1] = (char)(0x80u | ((unicodeChar >> 24u) & 0x3fu));
            dest[2] = (char)(0x80u | ((unicodeChar >> 18u) & 0x3fu));
            dest[3] = (char)(0x80u | ((unicodeChar >> 12u) & 0x3fu));
            dest[4] = (char)(0x80u | ((unicodeChar >> 6u) & 0x3fu));
            dest[5] = (char)(0x80u | (unicodeChar & 0x3fu));
            dest += 6;
        }
    }

#define GET_NEXT_CONTINUATION_BYTE(ptr) *(ptr); if ((uint8_t)*(ptr) < 0x80 || (uint8_t)*(ptr) >= 0xc0) return '?'; else ++(ptr);

    uint32_t String::DecodeUTF8(const char*& src)
    {
        if (src == nullptr)
            return 0;

        uint8_t char1 = *src++;

        // Check if we are in the middle of a UTF8 character
        if (char1 >= 0x80 && char1 < 0xc0)
        {
            while ((uint8_t)*src >= 0x80 && (uint8_t)*src < 0xc0)
                ++src;
            return '?';
        }

        if (char1 < 0x80)
        {
            return char1;
        }
        else if (char1 < 0xe0)
        {
            uint8_t char2 = GET_NEXT_CONTINUATION_BYTE(src);
            return (uint32_t)((char2 & 0x3fu) | ((char1 & 0x1fu) << 6u));
        }
        else if (char1 < 0xf0)
        {
            uint8_t char2 = GET_NEXT_CONTINUATION_BYTE(src);
            uint8_t char3 = GET_NEXT_CONTINUATION_BYTE(src);
            return (uint32_t)((char3 & 0x3fu) | ((char2 & 0x3fu) << 6u) | ((char1 & 0xfu) << 12u));
        }
        else if (char1 < 0xf8)
        {
            uint8_t char2 = GET_NEXT_CONTINUATION_BYTE(src);
            uint8_t char3 = GET_NEXT_CONTINUATION_BYTE(src);
            uint8_t char4 = GET_NEXT_CONTINUATION_BYTE(src);
            return (uint32_t)((char4 & 0x3fu) | ((char3 & 0x3fu) << 6u) | ((char2 & 0x3fu) << 12u) | ((char1 & 0x7u) << 18u));
        }
        else if (char1 < 0xfc)
        {
            uint8_t char2 = GET_NEXT_CONTINUATION_BYTE(src);
            uint8_t char3 = GET_NEXT_CONTINUATION_BYTE(src);
            uint8_t char4 = GET_NEXT_CONTINUATION_BYTE(src);
            uint8_t char5 = GET_NEXT_CONTINUATION_BYTE(src);
            return (uint32_t)((char5 & 0x3fu) | ((char4 & 0x3fu) << 6u) | ((char3 & 0x3fu) << 12u) | ((char2 & 0x3fu) << 18u) |
                ((char1 & 0x3u) << 24u));
        }
        else
        {
            uint8_t char2 = GET_NEXT_CONTINUATION_BYTE(src);
            uint8_t char3 = GET_NEXT_CONTINUATION_BYTE(src);
            uint8_t char4 = GET_NEXT_CONTINUATION_BYTE(src);
            uint8_t char5 = GET_NEXT_CONTINUATION_BYTE(src);
            uint8_t char6 = GET_NEXT_CONTINUATION_BYTE(src);
            return (uint32_t)((char6 & 0x3fu) | ((char5 & 0x3fu) << 6u) | ((char4 & 0x3fu) << 12u) | ((char3 & 0x3fu) << 18u) |
                ((char2 & 0x3fu) << 24u) | ((char1 & 0x1u) << 30u));
        }
    }

#ifdef _WIN32
    void String::EncodeUTF16(wchar_t*& dest, uint32_t unicodeChar)
    {
        if (unicodeChar < 0x10000)
        {
            *dest++ = (wchar_t)unicodeChar;
        }
        else
        {
            unicodeChar -= 0x10000;
            *dest++ = 0xd800 | ((unicodeChar >> 10) & 0x3ff);
            *dest++ = 0xdc00 | (unicodeChar & 0x3ff);
        }
    }

    uint32_t String::DecodeUTF16(const wchar_t*& src)
    {
        if (src == nullptr)
            return 0;

        uint16_t word1 = *src++;

        // Check if we are at a low surrogate
        if (word1 >= 0xdc00 && word1 < 0xe000)
        {
            while (*src >= 0xdc00 && *src < 0xe000)
                ++src;
            return '?';
        }

        if (word1 < 0xd800 || word1 >= 0xe000)
        {
            return word1;
        }
        else
        {
            uint16_t word2 = *src++;
            if (word2 < 0xdc00 || word2 >= 0xe000)
            {
                --src;
                return '?';
            }
            else
                return (((word1 & 0x3ff) << 10) | (word2 & 0x3ff)) + 0x10000;
        }
    }
#endif

    WString::WString(const String& str)
        : _length(0), _buffer(nullptr)
    {
#ifdef _WIN32
        uint32_t neededSize = 0;
        wchar_t temp[3];

        uint32_t byteOffset = 0;
        while (byteOffset < str.Length())
        {
            wchar_t* dest = temp;
            String::EncodeUTF16(dest, str.NextUTF8Char(byteOffset));
            neededSize += (uint32_t)(dest - temp);
        }

        Resize(neededSize);

        byteOffset = 0;
        wchar_t* dest = _buffer;
        while (byteOffset < str.Length())
        {
            String::EncodeUTF16(dest, str.NextUTF8Char(byteOffset));
        }
#else
        Resize(str.LengthUTF8());

        uint32_t byteOffset = 0;
        wchar_t* dest = _buffer;
        while (byteOffset < str.Length())
            *dest++ = (wchar_t)str.NextUTF8Char(byteOffset);
#endif
    }

    void WString::Resize(uint32_t newLength)
    {
        if (!newLength)
        {
            delete[] _buffer;
            _buffer = nullptr;
            _length = 0;
        }
        else
        {
            auto* newBuffer = new wchar_t[newLength + 1];
            if (_buffer)
            {
                uint32_t copyLength = _length < newLength ? _length : newLength;
                memcpy(newBuffer, _buffer, copyLength * sizeof(wchar_t));
                delete[] _buffer;
            }
            newBuffer[newLength] = 0;
            _buffer = newBuffer;
            _length = newLength;
        }
    }
}
