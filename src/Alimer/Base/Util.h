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
#include "../Base/StdHeaders.h"
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>
#include <type_traits>

namespace alimer
{
    /** Generates a new hash for the provided type using the default standard hasher and combines it with a previous hash. */
    template <class T>
    void HashCombine(std::size_t& seed, const T& v)
    {
        using HashType = typename std::conditional<std::is_enum<T>::value, EnumClassHash, std::hash<T>>::type;

        HashType hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    namespace str
    {
        namespace inner
        {
            template<typename T>
            void join_helper(std::ostringstream &stream, T &&t)
            {
                stream << std::forward<T>(t);
            }

            template<typename T, typename... Ts>
            void join_helper(std::ostringstream &stream, T &&t, Ts &&... ts)
            {
                stream << std::forward<T>(t);
                join_helper(stream, std::forward<Ts>(ts)...);
            }
        }

        template<typename... Ts>
        std::string join(Ts &&... ts)
        {
            std::ostringstream stream;
            inner::join_helper(stream, std::forward<Ts>(ts)...);
            return stream.str();
        }

        std::vector<std::string> split(const std::string &str, const char *delim);
        std::vector<std::string> split_no_empty(const std::string &str, const char *delim);
        std::string strip_whitespace(const std::string &str);
    }
}
