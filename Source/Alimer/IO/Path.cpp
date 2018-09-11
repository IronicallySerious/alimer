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

#include "../IO/Path.h"
#include "../Base/String.h"
#include "../Util/Util.h"
#include <algorithm>

namespace Alimer
{
	namespace Path
	{
		static uint32_t FindLastSlash(const String &str)
		{
#ifdef _WIN32
			auto index = str.FindLast("/\\");
#else
			auto index = str.FindLast('/');
#endif
			return index;
		}

		bool IsAbsolutePath(const String &path)
		{
			if (path.IsEmpty())
				return false;
			if (path.Front() == '/')
				return true;

#ifdef _WIN32
			{
				auto index = std::min(path.Find(":/"), path.Find(":\\"));
				if (index != String::NPOS)
					return true;
			}
#endif

			return path.Find("://") != String::NPOS;
		}

		bool IsRootPath(const String &path)
		{
			if (path.IsEmpty())
				return false;

			if (path.Front() == '/' && path.Length() == 1)
				return true;

#ifdef _WIN32
			{
				auto index = std::min(path.Find(":/"), path.Find(":\\"));
				if (index != String::NPOS && (index + 2) == path.Length())
					return true;
			}
#endif

			auto index = path.Find("://");
			return index != String::NPOS && (index + 3) == path.Length();
		}

        String Join(const String &base, const String &path)
		{
			if (base.IsEmpty())
				return path;
			if (path.IsEmpty())
				return base;

			if (IsAbsolutePath(path))
				return path;

            auto index = FindLastSlash(base);
			bool needSlash = index != base.Length() - 1;
            return String::Joined({ base, path }, needSlash ? "/" : "");
		}

        String GetBaseDir(const String &path)
		{
			if (path.IsEmpty())
				return "";

			if (IsRootPath(path))
				return path;

			auto index = FindLastSlash(path);
			if (index == String::NPOS)
				return ".";

			// Preserve the first slash.
			if (index == 0 && IsAbsolutePath(path))
				index++;

            String ret = path.Substring(0, index + 1);
            if (!IsRootPath(ret))
            {
                ret.Erase(ret.Length() - 1);
            }

			return ret;
		}

        String GetBaseName(const String &path)
		{
			if (path.IsEmpty())
				return String::EMPTY;

			auto index = FindLastSlash(path);
			if (index == String::NPOS)
				return path;

            String base = path.Substring(index + 1);
			return base;
		}

        String GetRelativePath(const String &base, const String &path)
		{
			return Path::Join(GetBaseDir(base), path);
		}

        String GetExtension(const String &path)
		{
			auto index = path.FindLast('.');
			if (index == String::NPOS)
				return String::EMPTY;

			return path.Substring(index + 1);
		}

		std::pair<String, String> ProtocolSplit(const String &path)
		{
			if (path.IsEmpty())
				return std::make_pair(String::EMPTY, String::EMPTY);

			auto index = path.Find("://");
			if (index == String::NPOS)
				return std::make_pair(String::EMPTY, path);

			return std::make_pair(path.Substring(0, index), path.Substring(index + 3));
		}
	}
}
