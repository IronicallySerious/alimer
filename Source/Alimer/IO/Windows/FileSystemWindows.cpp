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

	
	OSFileSystem::OSFileSystem(const string &basePath)
		: _basePath(basePath)
	{
		_path = Path::Join(FileSystem::GetExecutableFolder(), basePath);

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

	static bool EnsureDirectoryInner(const std::string &path)
	{
		if (Path::IsRootPath(path))
			return false;

		struct _stat s;
		if (::_stat(path.c_str(), &s) >= 0 && (s.st_mode & _S_IFDIR) != 0)
			return true;

		auto basedir = Path::GetBaseDir(path);
		if (!EnsureDirectoryInner(basedir))
			return false;

		if (!CreateDirectoryA(path.c_str(), nullptr))
			return GetLastError() == ERROR_ALREADY_EXISTS;
		return true;
	}

	static bool EnsureDirectory(const std::string &path)
	{
		std::string basedir = Path::GetBaseDir(path);
		return EnsureDirectoryInner(basedir);
	}


	class FileStream final : public Stream
	{
	public:
		FileStream(const string &path, StreamMode mode);
		~FileStream();

		bool CanSeek() const override { return _handle != nullptr; }

		size_t Read(void* dest, size_t size) override;
		void Write(const void* data, size_t size) override;

	private:
		HANDLE _handle = nullptr;
	};

	FileStream::FileStream(const string &path, StreamMode mode)
	{
		DWORD access = 0;
		DWORD disposition = 0;

		switch (mode)
		{
			case StreamMode::ReadOnly:
				access = GENERIC_READ;
				disposition = OPEN_EXISTING;
				break;

			case StreamMode::ReadWrite:
				if (!EnsureDirectory(path))
					throw runtime_error("Win32 Stream failed to create directory");
				access = GENERIC_READ | GENERIC_WRITE;
				disposition = OPEN_ALWAYS;
				break;

			case StreamMode::WriteOnly:
				if (!EnsureDirectory(path))
					throw runtime_error("Win32 Stream failed to create directory");
				access = GENERIC_READ | GENERIC_WRITE;
				disposition = CREATE_ALWAYS;
				break;
		}

		_handle = CreateFileA(path.c_str(), access, FILE_SHARE_READ, nullptr, disposition, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, INVALID_HANDLE_VALUE);
		if (_handle == INVALID_HANDLE_VALUE)
		{
			ALIMER_LOGERROR("Failed to open file: %s.\n", path.c_str());
			throw runtime_error("FileStream::FileStream()");
		}

		LARGE_INTEGER size;
		if (!GetFileSizeEx(_handle, &size))
		{
			throw runtime_error("[Win32] - GetFileSizeEx: failed");
		}

		_position = 0;
		_size = static_cast<size_t>(size.QuadPart);
	}

	FileStream::~FileStream()
	{
		if (_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(_handle);
			_handle = INVALID_HANDLE_VALUE;
		}

		_position = 0;
		_size = 0;
	}

	size_t FileStream::Read(void* dest, size_t size)
	{
		if (!CanRead())
		{
			ALIMER_LOGERROR("Cannot read for write only stream");
			return static_cast<size_t>(-1);
		}

		DWORD byteRead;
		if (!ReadFile(_handle, dest, static_cast<DWORD>(size), &byteRead, nullptr))
		{
			return static_cast<size_t>(-1);
		}

		_position += byteRead;
		return static_cast<size_t>(byteRead);
	}

	void FileStream::Write(const void* data, size_t size)
	{
		if (!size)
			return;

		DWORD byteWritten;
		if (!WriteFile(_handle, data, static_cast<DWORD>(size), &byteWritten, nullptr))
		{
			return;
		}

		_position += byteWritten;
		if (_position > _size)
			_size = _position;
	}

	std::unique_ptr<Stream> OSFileSystem::Open(const std::string &path, StreamMode mode)
	{
		if (mode == StreamMode::ReadOnly
			&& !FileExists(path))
		{
			return nullptr;
		}

		try
		{
			unique_ptr<FileStream> file(new FileStream(Path::Join(_basePath, path), mode));
			return file;
		}
		catch (const std::exception &e)
		{
			ALIMER_LOGERROR("OSFileSystem::Open(): %s\n", e.what());
			return {};
		}
	}

    string FileSystem::GetExecutableFolder()
    {
        wchar_t exeName[MAX_PATH];
        exeName[0] = 0;
        GetModuleFileNameW(nullptr, exeName, MAX_PATH);
        return Path::GetBaseDir(ToUniversalPath(ToMultibyte(exeName)));
    }
}
