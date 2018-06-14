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

#include "../AlimerConfig.h"
#include <cstdarg>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>

namespace Alimer
{
    /// String class.
    class ALIMER_API String final
    {
    public:
        using StorageType = std::vector<char>;
        using IndexType = StorageType::size_type;
        using Iterator = StorageType::iterator;
        using ConstIterator = StorageType::const_iterator;
        static constexpr IndexType npos = -1;

        String() {}
        String(const char* str) { SetData(str); }
        String(const char* begin, const char* end) { SetData(begin, end); }
        String(const String& str) { SetData(str.c_str()); }
        String(String&& str) { swap(str); }

        ~String() {}

        void clear() { _data.clear(); }
        const char* c_str() const { return _data.data(); }
        IndexType size() const { return _data.size() > 0 ? _data.size() - 1 : 0; }
        IndexType length() const { return _data.size() > 0 ? _data.size() - 1 : 0; }
        char* data() { return _data.data(); }
        const char* data() const { return _data.data(); }
        bool empty() const { return length() == 0; }

        void reserve(IndexType capacity) { _data.reserve(capacity); }
        void swap(String& other) { _data.swap(other._data); }
        void resize(IndexType size)
        {
            _data.resize(size + 1);
            _data.back() = '\0';
        }

        void shrink_to_fit() { _data.shrink_to_fit(); }

        Iterator begin() { return _data.begin(); }
        ConstIterator begin() const { return _data.begin(); }

        Iterator end() { return _data.end() - 1; }
        ConstIterator end() const { return _data.end() - 1; }

        String& operator=(const char* str) { return SetData(str); }
        String& operator=(const String& str) { return SetData(str.c_str()); }
        String& operator=(String&& str)
        {
            swap(str);
            return (*this);
        }

        bool operator==(const char* str) const { return strcmp(c_str(), str) == 0; }
        bool operator!=(const char* str) const { return strcmp(c_str(), str) != 0; }
        bool operator<(const char* str) const { return strcmp(c_str(), str) < 0; }
        bool operator>(const char* str) const { return strcmp(c_str(), str) > 0; }
        bool operator<=(const char* str) const { return strcmp(c_str(), str) <= 0; }
        bool operator>=(const char* str) const { return strcmp(c_str(), str) >= 0; }

        bool operator ==(const String& str) const { return strcmp(c_str(), str.c_str()) == 0; }
        bool operator !=(const String& str) const { return strcmp(c_str(), str.c_str()) != 0; }
        bool operator <(const String& str) const { return strcmp(c_str(), str.c_str()) < 0; }
        bool operator >(const String& str) const { return strcmp(c_str(), str.c_str()) > 0; }
        bool operator<=(const String& str) const { return strcmp(c_str(), str.c_str()) <= 0; }
        bool operator>=(const String& str) const { return strcmp(c_str(), str.c_str()) >= 0; }

        char& operator [](IndexType index)
        {
            assert(index < length());
            return _data[index];
        }

        /// Return const char at index.
        const char& operator [](IndexType index) const
        {
            assert(index < length());
            return _data[index];
        }

        /// Empty string.
        static const String EMPTY;

    private:
        String & SetData(const char* begin, const char* end = nullptr);

        StorageType _data;
    };
}


namespace std
{
    template<> struct less<Alimer::String>
    {
        bool operator()(const Alimer::String& v1, const Alimer::String& v2) const
        {
            return v1 < v2;
        }
    };
}
