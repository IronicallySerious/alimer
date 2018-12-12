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

#include "../IO/FileStream.h"
#include "../IO/FileSystem.h"
#include "../IO/Path.h"
#include "../Debug/Log.h"

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#   define INVALID_FILE_HANDLE INVALID_HANDLE_VALUE
#else
#   define INVALID_FILE_HANDLE nullptr
#endif

#include <cstdio>

namespace Alimer
{
#if !ALIMER_PLATFORM_WINDOWS && !ALIMER_PLATFORM_UWP
    static const char* openModes[] =
    {
        "rb",
        "wb",
        "r+b",
        "w+b"
    };
#endif

    static bool EnsureDirectoryExistsInner(const String &path)
    {
        if (Path::IsRootPath(path))
            return false;

        if (FileSystem::DirectoryExists(path))
            return true;

        auto basedir = Path::GetBaseDir(path);
        if (!EnsureDirectoryExistsInner(basedir))
            return false;

        if (!FileSystem::CreateDir(path))
            return false;

        return true;
    }

    static bool EnsureDirectoryExists(const String &path)
    {
        String basedir = Path::GetBaseDir(path);
        return EnsureDirectoryExistsInner(basedir);
    }

    FileStream::FileStream()
        : _mode(FileAccess::ReadOnly)
        , _canSeek(true)
        , _handle(INVALID_FILE_HANDLE)
    {
    }

    FileStream::FileStream(const String& fileName, FileAccess mode)
        : _mode(FileAccess::ReadOnly)
        , _canSeek(true)
        , _handle(INVALID_FILE_HANDLE)
    {
        Open(fileName, mode);
    }

    FileStream::~FileStream()
    {
        Close();
    }

    bool FileStream::Open(const String& fileName, FileAccess mode)
    {
        Close();

        if (fileName.IsEmpty())
            return false;

        if (mode == FileAccess::ReadOnly
            && !FileSystem::FileExists(fileName))
        {
            ALIMER_LOGERROR("Cannot open file for read as it doesn't exists");
            return false;
        }

        if (mode == FileAccess::ReadWrite
            || mode == FileAccess::WriteOnly)
        {
            if (!EnsureDirectoryExists(fileName))
            {
                ALIMER_LOGERROR("FileStream::Open - failed to create directory.");
                return false;
            }
        }

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        DWORD access = GENERIC_READ;
        DWORD disposition = 0;

        switch (mode)
        {
        case FileAccess::ReadOnly:
            disposition = OPEN_EXISTING;
            break;

        case FileAccess::ReadWrite:
            access |= GENERIC_WRITE;
            disposition = OPEN_ALWAYS;
            break;

        case FileAccess::WriteOnly:
            access |= GENERIC_WRITE;
            disposition = CREATE_ALWAYS;
            break;
        }

        _handle = CreateFileW(
            WString(fileName).CString(),
            access,
            FILE_SHARE_READ,
            nullptr,
            disposition,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, INVALID_HANDLE_VALUE
        );

        if (_handle == INVALID_HANDLE_VALUE)
        {
            ALIMER_LOGERRORF("Win32 - Failed to open file: '%s'.", fileName.CString());
        }

        if (mode != FileAccess::WriteOnly)
        {
            LARGE_INTEGER size;
            if (!GetFileSizeEx(_handle, &size))
            {
                ALIMER_LOGERROR("Win32 - GetFileSizeEx: failed");
            }

            _size = static_cast<uint64_t>(size.QuadPart);
        }
#else
        _handle = fopen(fileName.CString(), openModes[static_cast<uint32_t>(mode)]);
        // If file did not exist in readwrite mode, retry with write-update mode
        if (mode == FileAccess::ReadWrite
            && !_handle)
        {
            _handle = fopen(fileName.CString(), openModes[static_cast<uint32_t>(mode) + 1]);
        }

        if (!_handle)
            return false;

        if (mode != FileAccess::WriteOnly)
        {
            fseek((FILE*)_handle, 0, SEEK_END);
            _size = ftell((FILE*)_handle);
            fseek((FILE*)_handle, 0, SEEK_SET);
        }
#endif

        _name = fileName;
        _mode = mode;
        _position = 0;

        return true;
    }

    void FileStream::Close()
    {
        if (_handle != INVALID_FILE_HANDLE)
        {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
            ::CloseHandle(_handle);
#else
            fclose((FILE*)_handle);
#endif
            _handle = INVALID_FILE_HANDLE;
        }

        _position = 0;
        _size = 0;
    }

    void FileStream::Flush()
    {
        if (_handle != INVALID_FILE_HANDLE)
        {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
            if (::FlushFileBuffers(_handle) == S_FALSE)
            {
                //GetLastWin32Error()
            }
#else
            fflush((FILE*)_handle);
#endif
        }
    }

    bool FileStream::CanRead() const
    {
        return _handle != nullptr
            && (_mode == FileAccess::ReadOnly || _mode == FileAccess::ReadWrite);
    }

    bool FileStream::CanWrite() const
    {
        return _handle != nullptr
            && (_mode == FileAccess::WriteOnly || _mode == FileAccess::ReadWrite);
    }

    bool FileStream::CanSeek() const
    {
        return _canSeek;
    }

    uint64_t FileStream::Read(void* dest, uint64_t size)
    {
        if (!CanRead())
        {
            ALIMER_LOGERROR("Cannot read for write only stream");
            return 0;
        }

        if (size + _position > _size)
        {
            size = _size - _position;
        }

        if (!size)
            return 0;

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        DWORD byteRead;
        if (!ReadFile(_handle, dest, static_cast<DWORD>(size), &byteRead, nullptr))
        {
            return 0;
        }

        _position += byteRead;
        return static_cast<uint64_t>(byteRead);
#else
        uint64_t ret = fread(dest, size, 1, (FILE*)_handle);
        if (ret != 1)
        {
            // If error, return to the position where the read began
            fseek((FILE*)_handle, (long)_position, SEEK_SET);
            return 0;
        }

        _position += size;
        return size;
#endif
    }

    void FileStream::Write(const void* data, uint64_t size)
    {
        if (!_handle || _mode == FileAccess::ReadOnly)
            return;

        if (!size)
            return;

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        DWORD byteWritten;
        if (!WriteFile(_handle, data, static_cast<DWORD>(size), &byteWritten, nullptr))
        {
            return;
        }

        _position += byteWritten;
#else
        if (fwrite(data, size, 1, (FILE*)_handle) != 1)
        {
            // If error, return to the position where the write began
            fseek((FILE*)_handle, (long)_position, SEEK_SET);
            return;
        }

        _position += size;
#endif

        if (_position > _size)
        {
            _size = _position;
        }
    }

    uint64_t FileStream::Seek(int64_t offset, SeekOrigin origin)
    {
        if (_handle == INVALID_FILE_HANDLE)
            return 0;

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        DWORD moveMethod = 0;
        switch (origin)
        {
        case SeekOrigin::Begin:
            moveMethod = FILE_BEGIN;
            break;
        case SeekOrigin::Current:
            moveMethod = FILE_CURRENT;
            break;
        case SeekOrigin::End:
            moveMethod = FILE_END;
            break;
        default:
            break;
        }
        LARGE_INTEGER windowsOffset;
        windowsOffset.QuadPart = offset;
        if (!SetFilePointerEx(_handle, windowsOffset, &windowsOffset, moveMethod))
        {
            ALIMER_LOGERROR("Win32 - Seek failed");
            return static_cast<uint64_t>(-1);
        }
        _position = windowsOffset.QuadPart;
#else
        int seekMethod;
        switch (origin)
        {
        case SeekOrigin::Begin:
            seekMethod = SEEK_SET;
            break;
        case SeekOrigin::Current:
            seekMethod = SEEK_CUR;
            break;
        case SeekOrigin::End:
            seekMethod = SEEK_END;
            break;
        default:
            break;
        }
        fseek((FILE*)_handle, static_cast<long>(position), seekMethod);
        _position = ftell((FILE*)_handle);
#endif
        return _position;
    }

    bool FileStream::IsOpen() const
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        return _handle != INVALID_HANDLE_VALUE;
#else
        return _handle != nullptr;
#endif
    }
}
