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

#pragma once

#include "AlimerConfig.h"

#if defined(_WIN32) || defined(_WIN64)
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <malloc.h>
#   include <wchar.h>
#   include <windows.h>
#   define alloca _alloca
#elif ALIMER_PLATFORM_POSIX
#   ifdef BSD
#       include <stdlib.h>
#   else  
#       include <alloca.h>
#   endif
#   include <strings.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>

#define STACK_ARRAY(TYPE, LEN) static_cast<TYPE*>(::alloca((LEN) * sizeof(TYPE)))

namespace alimer
{
    static constexpr int CONVERSION_BUFFER_LENGTH = 128;
    static constexpr int MATRIX_CONVERSION_BUFFER_LENGTH = 256;

    using String = std::string;

    inline char tolowercase(char c) {
        return static_cast<char>(tolower(c));
    }

    inline char touppercase(char c) {
        return static_cast<char>(toupper(c));
    }

    class ALIMER_API StringUtils
    {
    public:
        template<typename... Ts>
        static std::string Join(Ts &&... ts)
        {
            std::ostringstream stream;
            join_helper(stream, std::forward<Ts>(ts)...);
            return stream.str();
        }

        /// Return a formatted string.
        static std::string Format(const char* format, ...);

        /// Return substrings split by a separator char. By default don't return empty strings.
        static std::vector<std::string> Split(const char* str, char separator, bool keepEmptyStrings = false);

        static bool StartsWith(const std::string& str, const std::string& pattern, bool caseSensitive = true);
        static bool EndsWith(const std::string& str, const std::string& pattern, bool caseSensitive = true);

        // Remove leading and trailing whitespaces.
        static std::string Trim(const std::string& str, bool left = true, bool right = true);
        static std::string LeftPad(char padding, unsigned length, std::string s);

        static std::string& ToLower(std::string& str);
        static std::string& ToUpper(std::string& str);

        static std::vector<std::string> Split(const std::string& str, const char *delim, bool allowEmpty);
        static std::string StripWhitespace(const std::string& str);
        static std::string Replace(const std::string& source, char oldValue, char newValue);
        static std::string Replace(const std::string& source, const std::string& oldValue, const std::string& newValue);

    private:
        template<typename T>
        static void join_helper(std::ostringstream &stream, T &&t)
        {
            stream << std::forward<T>(t);
        }

        template<typename T, typename... Ts>
        static void join_helper(std::ostringstream &stream, T &&t, Ts &&... ts)
        {
            stream << std::forward<T>(t);
            join_helper(stream, std::forward<Ts>(ts)...);
        }
    };

#if defined(_WIN32) || defined(_WIN64)
    inline std::wstring ToUtf16(const char* utf8, size_t len) {
        int len16 = ::MultiByteToWideChar(CP_UTF8, 0, utf8, static_cast<int>(len),
            nullptr, 0);
        wchar_t* ws = STACK_ARRAY(wchar_t, len16);
        ::MultiByteToWideChar(CP_UTF8, 0, utf8, static_cast<int>(len), ws, len16);
        return std::wstring(ws, len16);
    }

    inline std::wstring ToUtf16(const std::string& str) {
        return ToUtf16(str.data(), str.length());
    }

    inline std::string ToUtf8(const wchar_t* wide, size_t len) {
        int len8 = ::WideCharToMultiByte(CP_UTF8, 0, wide, static_cast<int>(len),
            nullptr, 0, nullptr, nullptr);
        char* ns = STACK_ARRAY(char, len8);
        ::WideCharToMultiByte(CP_UTF8, 0, wide, static_cast<int>(len), ns, len8,
            nullptr, nullptr);
        return std::string(ns, len8);
    }

    inline std::string ToUtf8(const wchar_t* wide) {
        return ToUtf8(wide, wcslen(wide));
    }

    inline std::string ToUtf8(const std::wstring& wstr) {
        return ToUtf8(wstr.data(), wstr.length());
    }
#endif  // defined(_WIN32) || defined(_WIN64)
}
