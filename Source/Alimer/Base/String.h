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

#include "../Base/Swap.h"
#include "../Base/Iterator.h"
#include <cassert>
#include <cstdarg>
#include <cstring>
#include <cctype>
#include <vector>
#include <sstream>
#include <algorithm>

namespace Alimer
{
    class String;
    class WString;
    class StringHash;

    static constexpr size_t CONVERSION_BUFFER_LENGTH = 128;

    /// String class.
    class ALIMER_API String
    {
    public:
        using Iterator = RandomAccessIterator<char>;
        using ConstIterator = RandomAccessConstIterator<char>;

        /// Construct empty.
        String() noexcept
            : _length(0)
            , _capacity(0)
            , _buffer(&END_ZERO)
        {
        }

        /// Construct from another string.
        String(const String& str)
            : _length(0)
            , _capacity(0)
            , _buffer(&END_ZERO)
        {
            *this = str;
        }

        /// Move-construct from another string.
        String(String && str) noexcept
            : _length(0)
            , _capacity(0)
            , _buffer(&END_ZERO)
        {
            Swap(str);
        }

        /// Construct from a C string.
        String(const char* str)   // NOLINT(google-explicit-constructor)
            : _length(0)
            , _capacity(0)
            , _buffer(&END_ZERO)
        {
            *this = str;
        }

        /// Construct from a C string.
        String(char* str) // NOLINT(google-explicit-constructor)
            : _length(0)
            , _capacity(0)
            , _buffer(&END_ZERO)
        {
            *this = (const char*)str;
        }

        /// Construct from a char array and length.
        String(const char* str, uint32_t length)
            : _length(0)
            , _capacity(0)
            , _buffer(&END_ZERO)
        {
            Resize(length);
            CopyChars(_buffer, str, length);
        }

        /// Construct from a null-terminated wide character array.
        explicit String(const wchar_t* str)
            : _length(0)
            , _capacity(0)
            , _buffer(&END_ZERO)
        {
            SetUTF8FromWChar(str);
        }

        /// Construct from a null-terminated wide character array.
        explicit String(wchar_t* str)
            : _length(0)
            , _capacity(0)
            , _buffer(&END_ZERO)
        {
            SetUTF8FromWChar(str);
        }

        /// Construct from a wide character string.
        explicit String(const WString& str);

        /// Construct from an integer.
        explicit String(int value);
        /// Construct from a short integer.
        explicit String(short value);
        /// Construct from a long integer.
        explicit String(long value);
        /// Construct from a long long integer.
        explicit String(long long value);
        /// Construct from an unsigned integer.
        explicit String(unsigned value);
        /// Construct from an unsigned short integer.
        explicit String(unsigned short value);
        /// Construct from an unsigned long integer.
        explicit String(unsigned long value);
        /// Construct from an unsigned long long integer.
        explicit String(unsigned long long value);
        /// Construct from a float.
        explicit String(float value);
        /// Construct from a double.
        explicit String(double value);
        /// Construct from a bool.
        explicit String(bool value);
        /// Construct from a character.
        explicit String(char value);
        /// Construct from a character and fill length.
        explicit String(char value, uint32_t length);

        /// Construct from a convertible value.
        template <class T> explicit String(const T& value)
            : _length(0)
            , _capacity(0)
            , _buffer(&END_ZERO)
        {
            *this = value.ToString();
        }

        /// Destruct.
        ~String()
        {
            if (_capacity)
                delete[] _buffer;
        }

        /// Assign a string.
        String& operator =(const String& rhs)
        {
            if (&rhs != this)
            {
                Resize(rhs._length);
                CopyChars(_buffer, rhs._buffer, rhs._length);
            }

            return *this;
        }

        /// Move-assign a string.
        String& operator =(String && rhs) noexcept
        {
            assert(&rhs != this);
            Swap(rhs);
            return *this;
        }

        /// Assign a C string.
        String& operator =(const char* rhs)
        {
            uint32_t rhsLength = CStringLength(rhs);
            Resize(rhsLength);
            CopyChars(_buffer, rhs, rhsLength);

            return *this;
        }

        /// Add-assign a string.
        String& operator +=(const String& rhs)
        {
            uint32_t oldLength = _length;
            Resize(_length + rhs._length);
            CopyChars(_buffer + oldLength, rhs._buffer, rhs._length);

            return *this;
        }

        /// Add-assign a C string.
        String& operator +=(const char* rhs)
        {
            uint32_t rhsLength = CStringLength(rhs);
            uint32_t oldLength = _length;
            Resize(_length + rhsLength);
            CopyChars(_buffer + oldLength, rhs, rhsLength);

            return *this;
        }

        /// Add-assign a character.
        String& operator +=(char rhs)
        {
            uint32_t oldLength = _length;
            Resize(_length + 1);
            _buffer[oldLength] = rhs;

            return *this;
        }

        /// Add-assign (concatenate as string) an integer.
        String& operator +=(int rhs);
        /// Add-assign (concatenate as string) a short integer.
        String& operator +=(short rhs);
        /// Add-assign (concatenate as string) a long integer.
        String& operator +=(long rhs);
        /// Add-assign (concatenate as string) a long long integer.
        String& operator +=(long long rhs);
        /// Add-assign (concatenate as string) an unsigned integer.
        String& operator +=(unsigned rhs);
        /// Add-assign (concatenate as string) a short unsigned integer.
        String& operator +=(unsigned short rhs);
        /// Add-assign (concatenate as string) a long unsigned integer.
        String& operator +=(unsigned long rhs);
        /// Add-assign (concatenate as string) a long long unsigned integer.
        String& operator +=(unsigned long long rhs);
        /// Add-assign (concatenate as string) a float.
        String& operator +=(float rhs);
        /// Add-assign (concatenate as string) a bool.
        String& operator +=(bool rhs);

        /// Add-assign (concatenate as string) an arbitrary type.
        template <class T> String& operator +=(const T& rhs) { return *this += rhs.ToString(); }

        /// Add a string.
        String operator +(const String& rhs) const
        {
            String ret;
            ret.Resize(_length + rhs._length);
            CopyChars(ret._buffer, _buffer, _length);
            CopyChars(ret._buffer + _length, rhs._buffer, rhs._length);

            return ret;
        }

        /// Add a C string.
        String operator +(const char* rhs) const
        {
            uint32_t rhsLength = CStringLength(rhs);
            String ret;
            ret.Resize(_length + rhsLength);
            CopyChars(ret._buffer, _buffer, _length);
            CopyChars(ret._buffer + _length, rhs, rhsLength);

            return ret;
        }

        /// Test for equality with another string.
        bool operator ==(const String& rhs) const { return strcmp(CString(), rhs.CString()) == 0; }

        /// Test for inequality with another string.
        bool operator !=(const String& rhs) const { return strcmp(CString(), rhs.CString()) != 0; }

        /// Test if string is less than another string.
        bool operator <(const String& rhs) const { return strcmp(CString(), rhs.CString()) < 0; }

        /// Test if string is greater than another string.
        bool operator >(const String& rhs) const { return strcmp(CString(), rhs.CString()) > 0; }

        /// Test for equality with a C string.
        bool operator ==(const char* rhs) const { return strcmp(CString(), rhs) == 0; }

        /// Test for inequality with a C string.
        bool operator !=(const char* rhs) const { return strcmp(CString(), rhs) != 0; }

        /// Test if string is less than a C string.
        bool operator <(const char* rhs) const { return strcmp(CString(), rhs) < 0; }

        /// Test if string is greater than a C string.
        bool operator >(const char* rhs) const { return strcmp(CString(), rhs) > 0; }

        /// Return char at index.
        char& operator [](uint32_t index)
        {
            assert(index < _length);
            return _buffer[index];
        }

        /// Return const char at index.
        const char& operator [](uint32_t index) const
        {
            assert(index < _length);
            return _buffer[index];
        }

        /// Return char at index.
        char& At(uint32_t index)
        {
            assert(index < _length);
            return _buffer[index];
        }

        /// Return const char at index.
        const char& At(uint32_t index) const
        {
            assert(index < _length);
            return _buffer[index];
        }

        /// Resize the string.
        void Resize(uint32_t newLength);
        /// Set new capacity.
        void Reserve(uint32_t newCapacity);
        /// Reallocate so that no extra memory is used.
        void Compact();
        /// Clear the string.
        void Clear();
        /// Swap with another string.
        void Swap(String& str);

        /// Return comparison result with a string.
        int Compare(const String& str, bool caseSensitive = true) const;
        /// Return comparison result with a C string.
        int Compare(const char* str, bool caseSensitive = true) const;
        /// Compare two C strings.
        static int Compare(const char* lhs, const char* rhs, bool caseSensitive);

        /// Replace all occurrences of a character.
        void Replace(char replaceThis, char replaceWith, bool caseSensitive = true);
        /// Replace all occurrences of a string.
        void Replace(const String& replaceThis, const String& replaceWith, bool caseSensitive = true);
        /// Replace a substring.
        void Replace(uint32_t pos, uint32_t length, const String& replaceWith);
        /// Replace a substring with a C string.
        void Replace(uint32_t pos, uint32_t length, const char* replaceWith);
        /// Replace a substring by iterators.
        Iterator Replace(const Iterator& start, const Iterator& end, const String& replaceWith);
        /// Return a string with all occurrences of a character replaced.
        String Replaced(char replaceThis, char replaceWith, bool caseSensitive = true) const;
        /// Return a string with all occurrences of a string replaced.
        String Replaced(const String& replaceThis, const String& replaceWith, bool caseSensitive = true) const;

        /// Append a string.
        String& Append(const String& str);
        /// Append a C string.
        String& Append(const char* str);
        /// Append a character.
        String& Append(char c);
        /// Append characters.
        String& Append(const char* str, uint32_t length);
        /// Append to string using formatting.
        String& AppendWithFormat(const char* formatString, ...);
        /// Append to string using variable arguments.
        String& AppendWithFormatArgs(const char* formatString, va_list args);

        /// Insert a string.
        void Insert(uint32_t pos, const String& str);
        /// Insert a character.
        void Insert(uint32_t pos, char c);
        /// Insert a string by iterator.
        Iterator Insert(const Iterator& dest, const String& str);
        /// Insert a string partially by iterators.
        Iterator Insert(const Iterator& dest, const Iterator& start, const Iterator& end);
        /// Insert a character by iterator.
        Iterator Insert(const Iterator& dest, char c);
        /// Erase a substring.
        void Erase(uint32_t pos, uint32_t length = 1);
        /// Erase a character by iterator.
        Iterator Erase(const Iterator& it);
        /// Erase a substring by iterators.
        Iterator Erase(const Iterator& start, const Iterator& end);

        /// Return a substring from position to end.
        String Substring(uint32_t pos) const;
        /// Return a substring with length from position.
        String Substring(uint32_t pos, uint32_t length) const;
        /// Return string with whitespace trimmed from the beginning and the end.
        String Trimmed(const String& chars = " \t\r\n") const;
        /// Return string in uppercase.
        String ToUpper() const;
        /// Return string in lowercase.
        String ToLower() const;

        /// Return substrings split by a separator char. By default don't return empty strings.
        std::vector<String> Split(char separator, bool keepEmptyStrings = false) const;
        /// Join substrings with a 'glue' string.
        void Join(const std::vector<String>& subStrings, const String& glue);
        /// Return index to the first occurrence of a string, or NPOS if not found.
        uint32_t Find(const String& str, uint32_t startPos = 0, bool caseSensitive = true) const;
        /// Return index to the first occurrence of a character, or NPOS if not found.
        uint32_t Find(char c, uint32_t startPos = 0, bool caseSensitive = true) const;
        /// Return index to the last occurrence of a string, or NPOS if not found.
        uint32_t FindLast(const String& str, uint32_t startPos = NPOS, bool caseSensitive = true) const;
        /// Return index to the last occurrence of a character, or NPOS if not found.
        uint32_t FindLast(char c, uint32_t startPos = NPOS, bool caseSensitive = true) const;
        /// Return whether starts with a string.
        bool StartsWith(const String& str, bool caseSensitive = true) const;
        /// Return whether ends with a string.
        bool EndsWith(const String& str, bool caseSensitive = true) const;

        /// Return whether contains a specific occurrence of a string.
        bool Contains(const String& str, bool caseSensitive = true) const { return Find(str, 0, caseSensitive) != NPOS; }

        /// Return whether contains a specific character.
        bool Contains(char c, bool caseSensitive = true) const { return Find(c, 0, caseSensitive) != NPOS; }

        /// Return substrings split by a separator char. By default don't return empty strings.
        static std::vector<String> Split(const char* str, char separator, bool keepEmptyStrings = false);
        /// Return a string by joining substrings with a 'glue' string.
        static String Joined(const std::vector<String>& subStrings, const String& glue);

        /// Return a formatted string.
        static String Format(const char* format, ...);

        /// Construct UTF8 content from Latin1.
        void SetUTF8FromLatin1(const char* str);
        /// Construct UTF8 content from wide characters.
        void SetUTF8FromWChar(const wchar_t* str);
        /// Calculate number of characters in UTF8 content.
        uint32_t LengthUTF8() const;
        /// Return byte offset to char in UTF8 content.
        uint32_t ByteOffsetUTF8(uint32_t index) const;
        /// Return next Unicode character from UTF8 content and increase byte offset.
        uint32_t NextUTF8Char(uint32_t& byteOffset) const;
        /// Return Unicode character at index from UTF8 content.
        uint32_t AtUTF8(uint32_t index) const;
        /// Replace Unicode character at index from UTF8 content.
        void ReplaceUTF8(uint32_t index, uint32_t unicodeChar);
        /// Append Unicode character at the end as UTF8.
        String& AppendUTF8(uint32_t unicodeChar);
        /// Return a UTF8 substring from position to end.
        String SubstringUTF8(uint32_t pos) const;
        /// Return a UTF8 substring with length from position.
        String SubstringUTF8(uint32_t pos, uint32_t length) const;

        /// Return iterator to the beginning.
        Iterator Begin() { return Iterator(_buffer); }

        /// Return const iterator to the beginning.
        ConstIterator Begin() const { return ConstIterator(_buffer); }

        /// Return iterator to the end.
        Iterator End() { return Iterator(_buffer + _length); }

        /// Return const iterator to the end.
        ConstIterator End() const { return ConstIterator(_buffer + _length); }

        /// Return first char, or 0 if empty.
        char Front() const { return _buffer[0]; }

        /// Return last char, or 0 if empty.
        char Back() const { return _length ? _buffer[_length - 1] : _buffer[0]; }

        /// Return the C string.
        const char* CString() const { return _buffer; }

        /// Return length.
        uint32_t Length() const { return _length; }

        /// Return buffer capacity.
        uint32_t Capacity() const { return _capacity; }

        /// Return whether the string is empty.
        bool IsEmpty() const { return _length == 0; }

        /// Return hash value for HashSet & HashMap.
        uint32_t ToHash() const
        {
            uint32_t hash = 0;
            const char* ptr = _buffer;
            while (*ptr)
            {
                hash = *ptr + (hash << 6u) + (hash << 16u) - hash;
                ++ptr;
            }

            return hash;
        }

        ///
        Iterator begin() { return Iterator(_buffer); }
        ConstIterator begin() const { return ConstIterator(_buffer); }

        Iterator end() { return Iterator(_buffer + _length); }
        ConstIterator end() const { return ConstIterator(_buffer + _length); }

        /// Return length of a C string.
        static uint32_t CStringLength(const char* str) { return str ? (uint32_t)strlen(str) : 0; }

        /// Encode Unicode character to UTF8. Pointer will be incremented.
        static void EncodeUTF8(char*& dest, uint32_t unicodeChar);
        /// Decode Unicode character from UTF8. Pointer will be incremented.
        static uint32_t DecodeUTF8(const char*& src);

#ifdef _WIN32
        /// Encode Unicode character to UTF16. Pointer will be incremented.
        static void EncodeUTF16(wchar_t*& dest, uint32_t unicodeChar);
        /// Decode Unicode character from UTF16. Pointer will be incremented.
        static uint32_t DecodeUTF16(const wchar_t*& src);
#endif

        /// Position for "not found."
        static constexpr uint32_t NPOS = 0xffffffff;
        /// Initial dynamic allocation size.
        static constexpr uint32_t MIN_CAPACITY = 8;
        /// Empty string.
        static const String EMPTY;

    private:
        /// Move a range of characters within the string.
        void MoveRange(uint32_t dest, uint32_t src, size_t count)
        {
            if (count)
            {
                memmove(_buffer + dest, _buffer + src, count);
            }
        }

        /// Copy chars from one buffer to another.
        static void CopyChars(char* dest, const char* src, size_t count)
        {
#ifdef _MSC_VER
            if (count) {
                memcpy(dest, src, count);
            }
#else
            char* end = dest + count;
            while (dest != end)
            {
                *dest = *src;
                ++dest;
                ++src;
            }
#endif
        }

        /// Replace a substring with another substring.
        void Replace(uint32_t pos, uint32_t length, const char* srcStart, uint32_t srcLength);

        /// String length.
        uint32_t _length;
        /// Capacity, zero if buffer not allocated.
        uint32_t _capacity;
        /// String buffer, point to &endZero if buffer is not allocated.
        char* _buffer;

        /// End zero for empty strings.
        static char END_ZERO;
    };

    /// Add a string to a C string.
    inline String operator +(const char* lhs, const String& rhs)
    {
        String ret(lhs);
        ret += rhs;
        return ret;
    }

    /// Add a string to a wide char C string.
    inline String operator +(const wchar_t* lhs, const String& rhs)
    {
        String ret(lhs);
        ret += rhs;
        return ret;
    }

    /// Wide character string. Only meant for converting from String and passing to the operating system where necessary.
    class ALIMER_API WString
    {
    public:
        /// Construct empty.
        WString()
            : _length(0), _buffer(nullptr)
        {
        }

        /// Construct from a string.
        explicit WString(const String& str);

        /// Destruct.
        ~WString()
        {
            delete[] _buffer;
        }

        /// Return char at index.
        wchar_t& operator [](uint32_t index)
        {
            assert(index < _length);
            return _buffer[index];
        }

        /// Return const char at index.
        const wchar_t& operator [](uint32_t index) const
        {
            assert(index < _length);
            return _buffer[index];
        }

        /// Return char at index.
        wchar_t& At(uint32_t index)
        {
            assert(index < _length);
            return _buffer[index];
        }

        /// Return const char at index.
        const wchar_t& At(uint32_t index) const
        {
            assert(index < _length);
            return _buffer[index];
        }

        /// Resize the string.
        void Resize(uint32_t newLength);

        /// Return whether the string is empty.
        bool IsEmpty() const { return _length == 0; }

        /// Return length.
        uint32_t Length() const { return _length; }

        /// Return character data.
        const wchar_t* CString() const { return _buffer; }

    private:
        /// String length.
        uint32_t _length;
        /// String buffer, null if not allocated.
        wchar_t* _buffer;
    };
}

namespace std
{
    template <> struct hash<Alimer::String>
    {
        size_t operator()(const Alimer::String& str) const
        {
            return str.ToHash();
        }
    };
}
