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

#include "WindowsFileSystem.h"
//#include "../../Util/Util.h"
#include "../../Core/String.h"
#include "../../IO/Path.h"
#include "../../Core/Log.h"

using namespace std;

namespace Alimer
{
    static bool EnsureDirectoryExistsInner(const string &path)
    {
        if (Path::IsRootPath(path))
            return false;

        if (DirectoryExists(path))
            return true;

        auto basedir = Path::GetBaseDir(path);
        if (!EnsureDirectoryExistsInner(basedir))
            return false;

        if (!CreateDirectoryA(path.c_str(), nullptr))
        {
            return GetLastError() == ERROR_ALREADY_EXISTS;
        }

        return true;
    }

    static bool EnsureDirectoryExists(const std::string &path)
    {
        string basedir = Path::GetBaseDir(path);
        return EnsureDirectoryExistsInner(basedir);
    }

    WindowsFileStream::WindowsFileStream(const string &path, StreamMode mode)
    {
        _name = path;
        DWORD access = 0;
        DWORD disposition = 0;

        switch (mode)
        {
        case StreamMode::ReadOnly:
            access = GENERIC_READ;
            disposition = OPEN_EXISTING;
            break;

        case StreamMode::ReadWrite:
            if (!EnsureDirectoryExists(path))
            {
                throw runtime_error("Win32 Stream failed to create directory.");
            }

            access = GENERIC_READ | GENERIC_WRITE;
            disposition = OPEN_ALWAYS;
            break;

        case StreamMode::WriteOnly:
            if (!EnsureDirectoryExists(path))
            {
                throw runtime_error("Win32 Stream failed to create directory.");
            }

            access = GENERIC_READ | GENERIC_WRITE;
            disposition = CREATE_ALWAYS;
            break;
        }

        _handle = CreateFileW(
            str::ToWide(path).c_str(),
            access,
            FILE_SHARE_READ,
            nullptr,
            disposition,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, INVALID_HANDLE_VALUE
        );
        if (_handle == INVALID_HANDLE_VALUE)
        {
            ALIMER_LOGERROR("Failed to open file: '{}'.", path.c_str());
            throw runtime_error("WindowsFileStream::WindowsFileStream()");
        }


        if (mode != StreamMode::WriteOnly)
        {
            LARGE_INTEGER size;
            if (!GetFileSizeEx(_handle, &size))
            {
                throw runtime_error("[Win32] - GetFileSizeEx: failed");
            }

            _size = static_cast<size_t>(size.QuadPart);
        }
    }

    WindowsFileStream::~WindowsFileStream()
    {
        if (_handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(_handle);
            _handle = INVALID_HANDLE_VALUE;
        }

        _position = 0;
        _size = 0;
    }

    size_t WindowsFileStream::Read(void* dest, size_t size)
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

    void WindowsFileStream::Write(const void* data, size_t size)
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

    OSFileSystemProtocol::OSFileSystemProtocol(const string &rootDirectory)
        : _rootDirectory(rootDirectory)
    {

    }

    OSFileSystemProtocol::~OSFileSystemProtocol()
    {

    }

    string OSFileSystemProtocol::GetFileSystemPath(const string& path)
    {
        return Path::Join(_rootDirectory, path);
    }

    unique_ptr<Stream> OSFileSystemProtocol::Open(const string &path, StreamMode mode)
    {
        try
        {
            unique_ptr<Stream> file(new WindowsFileStream(Path::Join(_rootDirectory, path), mode));
            return file;
        }
        catch (const std::exception &e)
        {
            ALIMER_LOGERROR("OSFileSystemProtocol::Open(): {}", e.what());
            return {};
        }
    }
}
