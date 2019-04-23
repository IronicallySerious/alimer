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

#include "foundation/StringUtils.h"
#include <algorithm>
#include <cctype> 
using namespace std;

namespace alimer
{
    string StringUtils::Trim(const string& str, bool left, bool right) {
        static const string whitespaces = " \t\r\n";

        string ret = str;
        if (right) {
            ret.erase(str.find_last_not_of(whitespaces) + 1); // trim right
        }

        if (left) {
            ret.erase(0, str.find_first_not_of(whitespaces)); // trim left
        }

        return ret;
    }

    string StringUtils::LeftPad(char padding, unsigned length, string s) {
        if (s.length() >= length)
            return s;

        return string(length - s.length(), padding) + s;
    }

    std::string StringUtils::Format(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        size_t len = std::vsnprintf(NULL, 0, format, args);
        va_end(args);

        std::vector<char> vec(len + 1);
        va_start(args, format);
        std::vsnprintf(&vec[0], len + 1, format, args);
        va_end(args);
        return &vec[0];
    }

    vector<string> StringUtils::Split(const char* str, char separator, bool keepEmptyStrings)
    {
        vector<string> ret;
        const char* strEnd = str + strlen(str);

        for (const char* splitEnd = str; splitEnd != strEnd; ++splitEnd)
        {
            if (*splitEnd == separator)
            {
                const ptrdiff_t splitLen = splitEnd - str;
                if (splitLen > 0 || keepEmptyStrings)
                {
                    ret.push_back(string(str, splitLen));
                }

                str = splitEnd + 1;
            }
        }

        const ptrdiff_t splitLen = strEnd - str;
        if (splitLen > 0 || keepEmptyStrings)
        {
            ret.push_back(string(str, splitLen));
        }

        return ret;
    }

    bool StringUtils::StartsWith(const string& str, const string& pattern, bool caseSensitive)
    {
        if (pattern.empty())
            return false;

        if (!caseSensitive)
        {
#if defined(_MSC_VER)
            return _strnicmp(str.c_str(), pattern.c_str(), pattern.size()) == 0;
#else
            return strnicmp(str.c_str(), pattern.c_str(), pattern.size()) == 0;
#endif
        }

        return strncmp(str.c_str(), pattern.c_str(), pattern.size()) == 0;
    }

    bool StringUtils::EndsWith(const string& str, const string& pattern, bool caseSensitive)
    {
        if (pattern.empty())
            return false;

        size_t offset = str.size() - pattern.size();

        if (!caseSensitive)
        {
#if defined(_MSC_VER)
            return _strnicmp(str.c_str() + offset, pattern.c_str(), pattern.size()) == 0;
#else
            return strnicmp(str.c_str() + offset, pattern.c_str(), pattern.size()) == 0;
#endif
        }

        return strncmp(str.c_str() + offset, pattern.c_str(), pattern.size()) == 0;
    }

    string& StringUtils::ToLower(string& str) {
        std::transform(str.begin(), str.end(), str.begin(), tolowercase);
        return str;
    }

    string& StringUtils::ToUpper(string& str) {
        std::transform(str.begin(), str.end(), str.begin(), touppercase);
        return str;
    }

    vector<string> StringUtils::Split(const string& str, const char *delim, bool allowEmpty) {
        if (str.empty())
            return {};

        vector<string> ret;
        size_t startIndex = 0;
        size_t index = 0;
        while ((index = str.find_first_of(delim, startIndex)) != string::npos)
        {
            if (allowEmpty || index > startIndex) {
                ret.push_back(str.substr(startIndex, index - startIndex));
            }

            startIndex = index + 1;

            if (allowEmpty && (index == str.size() - 1))
                ret.emplace_back();
        }

        if (startIndex < str.size()) {
            ret.push_back(str.substr(startIndex));
        }

        return ret;
    }

    string StringUtils::StripWhitespace(const string& str) {
        string ret;
        auto index = str.find_first_not_of(" \t");
        if (index == string::npos)
            return "";
        ret = str.substr(index, string::npos);
        index = ret.find_last_not_of(" \t");
        if (index != string::npos)
            return ret.substr(0, index + 1);

        return ret;
    }

    string StringUtils::Replace(const string& source, char oldValue, char newValue) {
        string result(source.size(), '\0');
        std::replace_copy(source.begin(), source.end(), result.begin(), oldValue, newValue);
        return result;
    }

    std::string StringUtils::Replace(const string& source, const string& oldValue, const string& newValue) {
        string result = source;
        string::size_type pos = 0;
        while (1)
        {
            pos = result.find(oldValue, pos);
            if (pos == string::npos)
                break;

            result.replace(pos, oldValue.size(), newValue);
            pos += newValue.size();
        }
        return result;
    }
}
