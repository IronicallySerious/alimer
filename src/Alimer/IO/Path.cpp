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

#include "../IO/Path.h"
#include <algorithm>
#include "foundation/StringUtils.h"
using namespace std;

namespace alimer
{
    static string GetCleanPath(const string& path)
    {
#ifdef _WIN32
        return StringUtils::Replace(path, '\\', '/');
#else
        return path;
#endif
    }

    static inline string::size_type FindLastSlash(const string &str)
    {
#ifdef _WIN32
        string::size_type index = str.find_last_of("/\\");
        if (index == string::npos)
        {
            index = str.find_last_of('/');
        }
        return index;
#else
        return str.FindLast('/');
#endif
    }

    bool Path::IsAbsolutePath(const string& path)
    {
        if (path.empty()) {
            return false;
        }

        if (path.front() == '/') {
            return true;
        }

#ifdef _WIN32
        {
            auto index = std::min(path.find(":/"), path.find(":\\"));
            if (index != string::npos)
                return true;
        }
#endif

        return path.find("://") != string::npos;
    }

    bool Path::IsRootPath(const string& path)
    {
        if (path.empty())
            return false;

        if (path.front() == '/' && path.length() == 1)
            return true;

#ifdef _WIN32
        {
            auto index = std::min(path.find(":/"), path.find(":\\"));
            if (index != string::npos && (index + 2) == path.length())
                return true;
        }
#endif

        auto index = path.find("://");
        return index != string::npos && (index + 3) == path.length();
    }

    string Path::Join(const string& base, const string& path)
    {
        if (base.empty())
            return path;
        if (path.empty())
            return base;

        if (IsAbsolutePath(path))
            return path;

        auto index = FindLastSlash(base);
        bool needSlash = index != base.length() - 1;
        return StringUtils::Join(base, needSlash ? "/" : "", path);
    }

    string Path::GetBaseDir(const string& path)
    {
        if (path.empty())
            return "";

        if (IsRootPath(path))
            return path;

        auto index = FindLastSlash(path);
        if (index == string::npos)
            return ".";

        // Preserve the first slash.
        if (index == 0 && IsAbsolutePath(path))
            index++;

        string ret = path.substr(0, index + 1);
        if (!IsRootPath(ret))
        {
            ret.erase(ret.length() - 1);
        }

        return ret;
    }

    string Path::GetBaseName(const string& path)
    {
        if (path.empty())
            return "";

        auto index = FindLastSlash(path);
        if (index == string::npos)
            return path;

        string base = path.substr(index + 1, string::npos);
        return base;
    }

    string Path::GetRelativePath(const string& base, const string& path)
    {
        return Path::Join(GetBaseDir(base), path);
    }

    string Path::GetExtension(const string& path, bool lowerCaseExtension)
    {
        string cleanPath = GetCleanPath(path);

        string::size_type extPos = cleanPath.find_last_of('.');
        string::size_type pathPos = cleanPath.find_last_of('/');

        string extension = "";
        if (extPos != string::npos
            && (pathPos == string::npos || extPos > pathPos))
        {
            extension = cleanPath.substr(extPos, string::npos);
            if (lowerCaseExtension)
            {
                StringUtils::ToLower(extension);
            }
        }

        return extension;
    }
}
