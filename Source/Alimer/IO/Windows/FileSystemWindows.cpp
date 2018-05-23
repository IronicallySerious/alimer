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

#include "FileSystemWindows.h"
#include "../../IO/Path.h"
#include "../../Util/Util.h"
#include "../../Core/Log.h"
using namespace std;

inline wstring ToWide(const string& str)
{
	int len;
	int slength = (int)str.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, nullptr, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
	wstring r(buf);
	delete[] buf;
	return r;
}

inline string ToMultibyte(const wstring& str)
{
	int len;
	int slength = (int)str.length() + 1;
	len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), slength, nullptr, 0, nullptr, nullptr);
	char* buf = new char[len];
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), slength, buf, len, nullptr, nullptr);
	string r(buf);
	delete[] buf;
	return r;
}

namespace Alimer
{
	string ToUniversalPath(const string& path)
	{
		return Util::Replace(path, "\\", "/");
	}

	string Path::GetExecutableFolder()
	{
		wchar_t exeName[MAX_PATH];
		exeName[0] = 0;
		GetModuleFileNameW(nullptr, exeName, MAX_PATH);
		return Path::GetBaseDir(ToUniversalPath(ToMultibyte(exeName)));
	}

	OSFileSystem::OSFileSystem(const string &basePath)
		: _basePath(basePath)
	{
		_path = Path::Join(Path::GetExecutableFolder(), basePath);

		static bool pathToExecutableSet = false;
		if (!pathToExecutableSet)
		{
			pathToExecutableSet = true;

			wchar_t exeName[MAX_PATH];
			exeName[0] = 0;
			GetModuleFileNameW(nullptr, exeName, MAX_PATH);
			std::wstring executablePath = ToWide(Path::GetBaseDir(ToMultibyte(exeName)));
			if (SetCurrentDirectoryW(executablePath.c_str()) == FALSE)
			{
				ALIMER_LOGERROR("[Win32] - Failed to change directory to executable directory");
				return;
			}
		}
	}

	OSFileSystem::~OSFileSystem()
	{

	}

	bool OSFileSystem::FileExists(const string &path)
	{
		string joined = Path::Join(_basePath, path);

		DWORD attributes = GetFileAttributesW(ToWide(joined).c_str());
		if (attributes != INVALID_FILE_ATTRIBUTES
			&& !(attributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			return true;
		}

		return false;
	}

	bool OSFileSystem::DirectoryExists(const std::string &path)
	{
		string joined = Path::Join(_basePath, path);

		DWORD attributes = GetFileAttributesW(ToWide(joined).c_str());
		if (attributes != INVALID_FILE_ATTRIBUTES
			&& attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			return true;
		}

		return false;
	}

	std::unique_ptr<Stream> OSFileSystem::Open(const std::string &path, StreamMode mode)
	{
		return nullptr;
	}
}
