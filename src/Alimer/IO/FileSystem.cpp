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
#include "../IO/FileSystem.h"
#include "../IO/Path.h"
#include "../Core/Log.h"

#include <sys/stat.h>
#include <cstdio>

#ifdef _WIN32
#   ifndef _MSC_VER
#       define _WIN32_IE 0x501
#   endif
#   include <windows.h>
#   include <shellapi.h>
#   include <direct.h>
#   include <shlobj.h>
#   include <sys/types.h>
#   include <sys/utime.h>
#else
#   include <dirent.h>
#   include <cerrno>
#   include <unistd.h>
#   include <utime.h>
#   include <sys/wait.h>
#   define MAX_PATH 256
#endif

#if defined(__APPLE__)
#   include <mach-o/dyld.h>
#endif

using namespace std;

namespace alimer
{
    class OSFileSystemProtocol final : public FileSystemProtocol
    {
    public:
        OSFileSystemProtocol(const string &rootDirectory)
            : _rootDirectory(rootDirectory)
        {

        }

        ~OSFileSystemProtocol() override = default;

        string GetFileSystemPath(const string& path) override
        {
            return Path::Join(_rootDirectory, path);
        }

        bool Exists(const string &path) override
        {
            string fullPath = Path::Join(_rootDirectory, path);
            return FileSystem::FileExists(fullPath);
        }

        unique_ptr<Stream> Open(const string &path, FileAccess mode) override
        {
            if (mode == FileAccess::ReadOnly
                && !Exists(path))
            {
                ALIMER_LOGERROR("Cannot open file '{}' as it doesn't exists", path);
                return {};
            }

            return unique_ptr<Stream>(new FileStream(Path::Join(_rootDirectory, path), mode));
        }

    protected:
        string _rootDirectory;
    };

    static std::pair<string, string> ProtocolSplit(const string &path)
    {
        if (path.empty())
            return std::make_pair(string(""), string(""));

        auto index = path.find("://");
        if (index == string::npos)
            return std::make_pair(string(""), path);

        return std::make_pair(path.substr(0, index), path.substr(index + 3, string::npos));
    }

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

    void FileSystem::RegisterProtocol(const std::string& name, FileSystemProtocol* protocol)
    {
        protocol->SetName(name);
        _protocols[name].reset(protocol);
    }

    FileSystemProtocol* FileSystem::GetProcotol(const string& name)
    {
        auto it = _protocols.find(name);
        if (name.empty()) {
            it = _protocols.find("file");
        }

        if (it != end(_protocols))
        {
            return it->second.get();
        }

        return nullptr;
    }

    bool FileSystem::Exists(const string& path)
    {
        auto paths = ProtocolSplit(path);
        auto *backend = GetProcotol(paths.first);
        if (!backend)
            return {};

        return backend->Exists(paths.second);
    }

    unique_ptr<Stream> FileSystem::Open(const string &path, FileAccess mode)
    {
        auto paths = ProtocolSplit(path);
        auto *backend = GetProcotol(paths.first);
        if (!backend)
            return {};

        return backend->Open(paths.second, mode);
    }

    string GetInternalPath(const string& path)
    {
        return StringUtils::Replace(path, '\\', '/');
    }

    string GetNativePath(const string& path)
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        return StringUtils::Replace(path, '/', '\\');
#else
        return path;
#endif
    }

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
    wstring GetWideNativePath(const string& path)
    {
        auto newPath = StringUtils::Replace(path, '/', '\\');
        return ToUtf16(newPath);
    }

    string GetUniversalPath(const string& path)
    {
        return StringUtils::Replace(path, '\\', '/');
    }
#endif

    string AddTrailingSlash(const string& path)
    {
        string ret = StringUtils::Replace(StringUtils::Trim(path), '\\', '/');
        if (!ret.empty() && ret.back() != '/')
            ret += '/';
        return ret;
    }

    string RemoveTrailingSlash(const string& path)
    {
        string ret = StringUtils::Replace(StringUtils::Trim(path), '\\', '/');
        if (!ret.empty() && ret.back() == '/') {
            ret.resize(ret.length() - 1);
        }
        return ret;
    }

    void SplitPath(const string& fullPath, string& pathName, string& fileName, string& extension, bool lowerCaseExtension)
    {
        string fullPathCopy = GetInternalPath(fullPath);

        string::size_type extPos = fullPathCopy.find_last_of('.');
        string::size_type pathPos = fullPathCopy.find_last_of('/');

        if (extPos != string::npos
            && (pathPos == string::npos || extPos > pathPos))
        {
            extension = fullPathCopy.substr(extPos, string::npos);
            if (lowerCaseExtension)
            {
                StringUtils::ToLower(extension);
            }

            fullPathCopy = fullPathCopy.substr(0, extPos);
        }
        else {
            extension.clear();
        }

        pathPos = fullPathCopy.find_last_of('/');
        if (pathPos != string::npos)
        {
            fileName = fullPathCopy.substr(pathPos + 1, string::npos);
            pathName = fullPathCopy.substr(0, pathPos + 1);
        }
        else
        {
            fileName = fullPathCopy;
            pathName.clear();
        }
    }

    string FileSystem::GetPath(const string& fullPath)
    {
        string path, file, extension;
        SplitPath(fullPath, path, file, extension);
        return path;
    }

    string FileSystem::GetFileName(const string& fullPath)
    {
        string path, file, extension;
        SplitPath(fullPath, path, file, extension);
        return file;
    }

    string FileSystem::GetExtension(const string& fullPath, bool lowercaseExtension)
    {
        string path, file, extension;
        SplitPath(fullPath, path, file, extension, lowercaseExtension);
        return extension;
    }

    string FileSystem::GetFileNameAndExtension(const string& fileName, bool lowercaseExtension)
    {
        string path, file, extension;
        SplitPath(fileName, path, file, extension, lowercaseExtension);
        return file + extension;
    }

    string GetParentPath(const string& path)
    {
        string::size_type pos = RemoveTrailingSlash(path).find_last_of('/');
        if (pos != string::npos) {
            return path.substr(0, pos + 1);
        }

        return string("");
    }

    bool IsAbsolutePath(const string& pathName)
    {
        if (pathName.empty())
            return false;

        string path = GetInternalPath(pathName);

        if (path[0] == '/')
            return true;

#ifdef _WIN32
        if (path.length() > 1 && isalpha(path[0]) && path[1] == ':')
            return true;
#endif

        return false;
    }

    // File
    bool FileSystem::FileExists(const string& fileName)
    {
        string fixedName = GetNativePath(RemoveTrailingSlash(fileName));

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        DWORD attributes = GetFileAttributesW(ToUtf16(fixedName).c_str());
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

    bool FileSystem::DirectoryExists(const string& path)
    {
#ifndef _WIN32
        // Always return true for the root directory
        if (path == "/")
            return true;
#endif

        string fixedName = GetNativePath(RemoveTrailingSlash(path));

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        DWORD attributes = GetFileAttributesW(ToUtf16(fixedName).c_str());
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

    bool FileSystem::CreateDir(const string& path)
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        wstring normalizePath = GetWideNativePath(RemoveTrailingSlash(path));
        return CreateDirectoryW(normalizePath.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
#else
        return mkdir(RemoveTrailingSlash(path).CString(), S_IRWXU) == 0 || errno == EEXIST;
#endif
    }

    string FileSystem::GetCurrentDir()
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        wchar_t path[MAX_PATH];
        path[0] = 0;
        GetCurrentDirectoryW(MAX_PATH, path);
        return ToUtf8(path);
#else
        char path[MAX_PATH];
        path[0] = 0;
        getcwd(path, MAX_PATH);
        return string(path);
#endif
    }

    string FileSystem::GetExecutableFolder()
    {
        string result = "";
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        wchar_t exeName[MAX_PATH];
        exeName[0] = 0;
        GetModuleFileNameW(nullptr, exeName, MAX_PATH);
        result = FileSystem::GetPath(GetUniversalPath(ToUtf8(exeName)));
#elif defined(__APPLE__)
        char exeName[MAX_PATH];
        memset(exeName, 0, MAX_PATH);
        unsigned size = MAX_PATH;
        _NSGetExecutablePath(exeName, &size);
        result = FileSystem::GetPath(string(exeName));
#elif defined(__linux__)
        char exeName[MAX_PATH];
        memset(exeName, 0, MAX_PATH);
        pid_t pid = getpid();
        string link = "/proc/" + string(pid) + "/exe";
        readlink(link.c_str(), exeName, MAX_PATH);
        result = FileSystem::GetPath(string(exeName));
#endif
        if (!result.empty())
        {
            // Sanitate /./ construct away
            result = StringUtils::Replace(result, "/./", "/");
        }

        return result;
    }

#ifdef _WIN32
    void ScanDirInternal(
        std::vector<string>& result, string path, const string& startPath,
        const string& filter, ScanDirFlags flags, bool recursive)
    {
        path = AddTrailingSlash(path);
        string deltaPath;
        if (path.length() > startPath.length())
            deltaPath = path.substr(startPath.length(), string::npos);

        string filterExtension = filter.substr(filter.find_last_of('.'));
        if (filterExtension.find('*') != string::npos)
        {
            filterExtension.clear();
        }

        WIN32_FIND_DATAW info;
        path = path + "*";
        HANDLE handle = FindFirstFileW(ToUtf16(path).c_str(), &info);
        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                string fileName = ToUtf8(info.cFileName);
                if (!fileName.empty())
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
                        if (filterExtension.empty()
                            || StringUtils::EndsWith(fileName, filterExtension))
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
        std::vector<string>& result,
        const string& pathName,
        const string& filter,
        ScanDirFlags flags, bool recursive)
    {
        result.clear();

        string initialPath = AddTrailingSlash(pathName);
        ScanDirInternal(result, initialPath, initialPath, filter, flags, recursive);
    }
#endif
}
