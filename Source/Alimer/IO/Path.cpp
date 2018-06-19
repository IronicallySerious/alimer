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
#include "../Core/String.h"
#include "../Util/Util.h"
#include <algorithm>
using namespace std;

namespace Alimer
{
	namespace Path
	{
		static size_t FindLastSlash(const string &str)
		{
#ifdef _WIN32
			auto index = str.find_last_of("/\\");
#else
			auto index = str.find_last_of('/');
#endif
			return index;
		}

		bool IsAbsolutePath(const string &path)
		{
			if (path.empty())
				return false;
			if (path.front() == '/')
				return true;

#ifdef _WIN32
			{
				auto index = std::min(path.find(":/"), path.find(":\\"));
				if (index != string::npos)
					return true;
			}
#endif

			return path.find("://") != string::npos;
		}

		bool IsRootPath(const string &path)
		{
			if (path.empty())
				return false;

			if (path.front() == '/' && path.size() == 1)
				return true;

#ifdef _WIN32
			{
				auto index = std::min(path.find(":/"), path.find(":\\"));
				if (index != string::npos && (index + 2) == path.size())
					return true;
			}
#endif

			auto index = path.find("://");
			return index != string::npos && (index + 3) == path.size();
		}

		string Join(const string &base, const string &path)
		{
			if (base.empty())
				return path;
			if (path.empty())
				return base;

			if (IsAbsolutePath(path))
				return path;

			size_t index = FindLastSlash(base);
			bool needSlash = index != base.size() - 1;
			return str::Join(base, needSlash ? "/" : "", path);
		}

		string GetBaseDir(const string &path)
		{
			if (path.empty())
				return "";

			if (IsRootPath(path))
				return path;

			size_t index = FindLastSlash(path);
			if (index == string::npos)
				return ".";

			// Preserve the first slash.
			if (index == 0 && IsAbsolutePath(path))
				index++;

			string ret = path.substr(0, index + 1);
			if (!IsRootPath(ret))
				ret.pop_back();
			return ret;
		}

		string GetBaseName(const string &path)
		{
			if (path.empty())
				return "";

			size_t index = FindLastSlash(path);
			if (index == string::npos)
				return path;

			string base = path.substr(index + 1, string::npos);
			return base;
		}

		string GetRelativePath(const string &base, const string &path)
		{
			return Path::Join(GetBaseDir(base), path);
		}

		string GetExtension(const string &path)
		{
			auto index = path.find_last_of('.');
			if (index == string::npos)
				return "";

			return path.substr(index + 1, string::npos);
		}

		pair<string, string> ProtocolSplit(const string &path)
		{
			if (path.empty())
				return make_pair(string(""), string(""));

			size_t index = path.find("://");
			if (index == string::npos)
				return make_pair(string(""), path);

			return make_pair(path.substr(0, index), path.substr(index + 3, string::npos));
		}
	}
}
