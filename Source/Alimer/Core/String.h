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

#include "../Core/Log.h"
#include <cstdarg>
#include <cstring>
#include <cctype>
#include <locale>
#include <codecvt>
#include <string>
#include <sstream>
#include <vector>
#include <type_traits>
#include <algorithm>
#define FMT_NO_FMT_STRING_ALIAS
#include <fmt/format.h>

namespace Alimer
{
    template <typename T>
    struct EnumNames {
    };

    namespace str
    {
        namespace inner
        {
            template<typename T>
            void JoinHelper(std::ostringstream &stream, T &&t)
            {
                stream << std::forward<T>(t);
            }

            template<typename T, typename... Ts>
            void JoinHelper(std::ostringstream &stream, T &&t, Ts &&... ts)
            {
                stream << std::forward<T>(t);
                JoinHelper(stream, std::forward<Ts>(ts)...);
            }
        }

        template<typename... Ts>
        std::string Join(Ts &&... ts)
        {
            std::ostringstream stream;
            inner::JoinHelper(stream, std::forward<Ts>(ts)...);
            return stream.str();
        }

        template<typename T, typename = typename std::enable_if<std::is_enum<T>::value>::type>
        static std::string ToString(const T& v)
        {
            return EnumNames<T>()()[ecast(v)];
        }

        template<typename T, typename = typename std::enable_if<std::is_enum<T>::value>::type>
        static T FromString(const std::string& str)
        {
            EnumNames<T> n;
            auto names = n();
            auto res = std::find_if(std::begin(names), std::end(names), [&](const char* v) { return str == v; });
            if (res == std::end(names))
            {
                ALIMER_LOGCRITICAL(
                    "String '%s' does not exist in enum '%s'.",
                    str.c_str(),
                    typeid(T).name());
            }

            return T(res - std::begin(names));
        }

        inline std::string FromWide(const std::wstring& ws)
        {
            return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(ws);
        }

        inline std::wstring ToWide(const std::string& s)
        {
            return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(s);
        }
    }
}
