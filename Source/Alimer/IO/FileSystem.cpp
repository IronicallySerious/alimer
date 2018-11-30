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

#include "../IO/FileSystem.h"
#include "../IO/Path.h"
#include "../Base/String.h"
#include "../Debug/Log.h"

#include <sys/stat.h>
#include <cstdio>

#ifdef _WIN32
#ifndef _MSC_VER
#define _WIN32_IE 0x501
#endif
#include <windows.h>
#include <shellapi.h>
#include <direct.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/utime.h>
#else
#include <dirent.h>
#include <cerrno>
#include <unistd.h>
#include <utime.h>
#include <sys/wait.h>
#define MAX_PATH 256
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace Alimer
{
    class OSFileSystemProtocol final : public FileSystemProtocol
    {
    public:
        OSFileSystemProtocol(const String &rootDirectory)
            : _rootDirectory(rootDirectory)
        {

        }

        ~OSFileSystemProtocol() override = default;

        String GetFileSystemPath(const String& path) override
        {
            return Path::Join(_rootDirectory, path);
        }

        bool Exists(const String &path) override
        {
            String fullPath = Path::Join(_rootDirectory, path);
            return FileSystem::FileExists(fullPath);
        }

        UniquePtr<Stream> Open(const String &path, FileAccess mode) override
        {
            if (mode == FileAccess::ReadOnly
                && !Exists(path))
            {
                ALIMER_LOGERRORF("Cannot open file '%s' as it doesn't exists", path.CString());
                return {};
            }

            return UniquePtr<Stream>(new FileStream(Path::Join(_rootDirectory, path), mode));
        }

    protected:
        String _rootDirectory;
    };

    FileSystem::FileSystem()
    {
        // Register default file protocol.
        RegisterProtocol("file", new OSFileSystemProtocol("."));

        // Lookup assets folder at executable path first.
        if (DirectoryExists("assets"))
        {
            RegisterProtocol("assets", new OSFileSystemProtocol("assets"));
        }
#ifdef ALIMER_DEFAULT_ASSETS_DIRECTORY
        else
        {

            const char *assetsDir = ALIMER_DEFAULT_ASSETS_DIRECTORY;
            if (assetsDir)
            {
                RegisterProtocol("assets", new OSFileSystemProtocol(assetsDir));
            }
        }
#endif // ALIMER_DEFAULT_ASSETS_DIRECTORY
    }

    FileSystem &FileSystem::Get()
    {
        static FileSystem fs;
        return fs;
    }

    void FileSystem::RegisterProtocol(const String &name, FileSystemProtocol* protocol)
    {
        protocol->SetName(name);
        _protocols[name].Reset(protocol);
    }

    FileSystemProtocol* FileSystem::GetProcotol(const String &name)
    {
        auto it = _protocols.find(name);
        if (name.IsEmpty())
            it = _protocols.find("file");

        if (it != end(_protocols))
            return it->second.Get();

        return nullptr;
    }

    bool FileSystem::Exists(const String &path)
    {
        auto paths = Path::ProtocolSplit(path);
        auto *backend = GetProcotol(paths.first);
        if (!backend)
            return {};

        return backend->Exists(paths.second);
    }

    UniquePtr<Stream> FileSystem::Open(const String &path, FileAccess mode)
    {
        auto paths = Path::ProtocolSplit(path);
        auto *backend = GetProcotol(paths.first);
        if (!backend)
            return {};

        return backend->Open(paths.second, mode);
    }

    String GetInternalPath(const String& path)
    {
        return path.Replaced('\\', '/');
    }

    String GetNativePath(const String& path)
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        return path.Replaced('/', '\\');
#else
        return path;
#endif
    }

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
    WString GetWideNativePath(const String& path)
    {
        return WString(path.Replaced('/', '\\'));
    }

    String GetUniversalPath(const String& path)
    {
        return path.Replaced("\\", "/");
    }
#endif

    String AddTrailingSlash(const String& path)
    {
        String ret = path.Trimmed();
        ret.Replace('\\', '/');
        if (!ret.IsEmpty() && ret.Back() != '/')
            ret += '/';
        return ret;
    }

    String RemoveTrailingSlash(const String& path)
    {
        String ret = path.Trimmed();
        ret.Replace('\\', '/');
        if (!ret.IsEmpty() && ret.Back() == '/')
            ret.Resize(ret.Length() - 1);
        return ret;
    }

    void SplitPath(const String& fullPath, String& pathName, String& fileName, String& extension, bool lowerCaseExtension)
    {
        String fullPathCopy = GetInternalPath(fullPath);

        uint32_t extPos = fullPathCopy.FindLast('.');
        uint32_t pathPos = fullPathCopy.FindLast('/');

        if (extPos != String::NPOS
            && (pathPos == String::NPOS || extPos > pathPos))
        {
            extension = fullPathCopy.Substring(extPos);
            if (lowerCaseExtension)
            {
                extension = extension.ToLower();
            }

            fullPathCopy = fullPathCopy.Substring(0, extPos);
        }
        else {
            extension.Clear();
        }

        pathPos = fullPathCopy.FindLast('/');
        if (pathPos != String::NPOS)
        {
            fileName = fullPathCopy.Substring(pathPos + 1);
            pathName = fullPathCopy.Substring(0, pathPos + 1);
        }
        else
        {
            fileName = fullPathCopy;
            pathName.Clear();
        }
    }

    String FileSystem::GetPath(const String& fullPath)
    {
        String path, file, extension;
        SplitPath(fullPath, path, file, extension);
        return path;
    }

    String FileSystem::GetFileName(const String& fullPath)
    {
        String path, file, extension;
        SplitPath(fullPath, path, file, extension);
        return file;
    }

    String FileSystem::GetExtension(const String& fullPath, bool lowercaseExtension)
    {
        String path, file, extension;
        SplitPath(fullPath, path, file, extension, lowercaseExtension);
        return extension;
    }

    String FileSystem::GetFileNameAndExtension(const String& fileName, bool lowercaseExtension)
    {
        String path, file, extension;
        SplitPath(fileName, path, file, extension, lowercaseExtension);
        return file + extension;
    }

    String GetParentPath(const String& path)
    {
        uint32_t pos = RemoveTrailingSlash(path).FindLast('/');
        if (pos != String::NPOS)
            return path.Substring(0, pos + 1);

        return String();
    }

    bool IsAbsolutePath(const String& pathName)
    {
        if (pathName.IsEmpty())
            return false;

        String path = GetInternalPath(pathName);

        if (path[0] == '/')
            return true;

#ifdef _WIN32
        if (path.Length() > 1 && isalpha(path[0]) && path[1] == ':')
            return true;
#endif

        return false;
    }

    // File
    bool FileSystem::FileExists(const String& fileName)
    {
        String fixedName = GetNativePath(RemoveTrailingSlash(fileName));

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        DWORD attributes = GetFileAttributesW(WString(fixedName).CString());
        if (attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            return false;
        }
#else
        struct stat st {};
        if (stat(fixedName.CString(), &st) || st.st_mode & S_IFDIR)
        {
            return false;
        }
#endif
        return true;
    }

    bool FileSystem::DirectoryExists(const String& path)
    {
#ifndef _WIN32
        // Always return true for the root directory
        if (path == "/")
            return true;
#endif

        String fixedName = GetNativePath(RemoveTrailingSlash(path));

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        DWORD attributes = GetFileAttributesW(WString(fixedName).CString());
        if (attributes == INVALID_FILE_ATTRIBUTES
            || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            return false;
        }
#else
        struct stat st;
        if (stat(fixedName.CString(), &st) || !(st.st_mode & S_IFDIR))
            return false;
#endif
        return true;
    }

    bool FileSystem::CreateDir(const String& path)
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        WString normalizePath = GetWideNativePath(RemoveTrailingSlash(path));
        return CreateDirectoryW(normalizePath.CString(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
#else
        return mkdir(RemoveTrailingSlash(path).CString(), S_IRWXU) == 0 || errno == EEXIST;
#endif
    }

    String FileSystem::GetCurrentDir()
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        wchar_t path[MAX_PATH];
        path[0] = 0;
        GetCurrentDirectoryW(MAX_PATH, path);
        return String(path);
#else
        char path[MAX_PATH];
        path[0] = 0;
        getcwd(path, MAX_PATH);
        return String(path);
#endif
    }

    String FileSystem::GetExecutableFolder()
    {
        String result = String::EMPTY;
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        wchar_t exeName[MAX_PATH];
        exeName[0] = 0;
        GetModuleFileNameW(nullptr, exeName, MAX_PATH);
        result = FileSystem::GetPath(GetUniversalPath(String(exeName)));
#elif defined(__APPLE__)
        char exeName[MAX_PATH];
        memset(exeName, 0, MAX_PATH);
        unsigned size = MAX_PATH;
        _NSGetExecutablePath(exeName, &size);
        result = FileSystem::GetPath(String(exeName));
#elif defined(__linux__)
        char exeName[MAX_PATH];
        memset(exeName, 0, MAX_PATH);
        pid_t pid = getpid();
        String link = "/proc/" + String(pid) + "/exe";
        readlink(link.CString(), exeName, MAX_PATH);
        result = FileSystem::GetPath(String(exeName));
#endif
        if (!result.IsEmpty())
        {
            // Sanitate /./ construct away
            result.Replace("/./", "/");
        }

        return result;
    }

#ifdef _WIN32
    void ScanDirInternal(
        std::vector<String>& result, String path, const String& startPath,
        const String& filter, ScanDirFlags flags, bool recursive)
    {
        path = AddTrailingSlash(path);
        String deltaPath;
        if (path.Length() > startPath.Length())
            deltaPath = path.Substring(startPath.Length());

        String filterExtension = filter.Substring(filter.FindLast('.'));
        if (filterExtension.Find('*') != String::NPOS)
        {
            filterExtension.Clear();
        }

        WIN32_FIND_DATAW info;
        HANDLE handle = FindFirstFileW(WString(path + "*").CString(), &info);
        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                String fileName(info.cFileName);
                if (!fileName.IsEmpty())
                {
                    if (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN && !any(flags & ScanDirFlags::Hidden))
                        continue;

                    if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if (any(flags & ScanDirFlags::Directories))
                            result.push_back(deltaPath + fileName);
                        if (recursive && fileName != "." && fileName != "..")
                        {
                            ScanDirInternal(result, path + fileName, startPath, filter, flags, recursive);
                        }
                    }
                    else if (any(flags & ScanDirFlags::Files))
                    {
                        if (filterExtension.IsEmpty()
                            || fileName.EndsWith(filterExtension))
                        {
                            result.push_back(deltaPath + fileName);
                        }
                    }
                }
            } while (FindNextFileW(handle, &info));

            FindClose(handle);
        }
    }

    void ScanDirectory(
        std::vector<String>& result,
        const String& pathName,
        const String& filter,
        ScanDirFlags flags, bool recursive)
    {
        result.clear();

        String initialPath = AddTrailingSlash(pathName);
        ScanDirInternal(result, initialPath, initialPath, filter, flags, recursive);
    }
#endif
}
