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

#include "../Core/String.h"
#include <nlohmann/json.hpp>

namespace Alimer
{
    const String String::EMPTY;

    String& String::SetData(const char* begin, const char* end)
    {
        if (begin)
        {
            if (!end)
            {
                size_t length = strlen(begin) + 1;
                _data.reserve(length);
                _data.clear();
                _data.insert(_data.begin(), begin, begin + length);
            }
            else
            {
                _data.clear();
                _data.insert(_data.begin(), begin, end);
                _data.emplace_back('\0');
            }
        }
        else
        {
            _data.clear();
        }

        return *this;
    }
}

namespace nlohmann {
    template <>
    struct adl_serializer<Alimer::String>
    {
        static void to_json(json& j, const Alimer::String& str) {
            j = str.c_str();
        }

        static void from_json(const json& j, Alimer::String& str) {
            if (j.is_null()) {
                str = Alimer::String::EMPTY;
            }
            else {
                str = j.get<std::string>().c_str();
            }
        }
    };
}
