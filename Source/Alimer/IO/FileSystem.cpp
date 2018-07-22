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
#include "../Util/Util.h"
#include "../Core/String.h"
#include "../Core/Log.h"

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#include "../IO/Windows/WindowsFileSystem.h"
#endif

using namespace std;

namespace Alimer
{
    FileSystem::FileSystem()
    {
        // Register default file protocol.
        RegisterProtocol("file", unique_ptr<FileSystemProtocol>(new OSFileSystemProtocol(".")));

#ifdef ALIMER_DEFAULT_ASSETS_DIRECTORY
        const char *assetsDir = ALIMER_DEFAULT_ASSETS_DIRECTORY;
        if (assetsDir)
        {
            RegisterProtocol("assets", unique_ptr<FileSystemProtocol>(new OSFileSystemProtocol(assetsDir)));
        }
#else
        // Lookup assets folder
        if (DirectoryExists("assets"))
        {
            RegisterProtocol("assets", unique_ptr<FileSystemProtocol>(new OSFileSystemProtocol("assets")));
        }
#endif // ALIMER_DEFAULT_ASSETS_DIRECTORY
    }

    FileSystem &FileSystem::Get()
    {
        static FileSystem fs;
        return fs;
    }

    void FileSystem::RegisterProtocol(const string &name, unique_ptr<FileSystemProtocol> protocol)
    {
        protocol->SetName(name);
        // Dispatch event.
        _protocols[name] = move(protocol);
    }

    FileSystemProtocol* FileSystem::GetProcotol(const std::string &name)
    {
        auto it = _protocols.find(name);
        if (name.empty())
            it = _protocols.find("file");

        if (it != end(_protocols))
            return it->second.get();

        return nullptr;
    }

    unique_ptr<Stream> FileSystem::Open(const std::string &path, StreamMode mode)
    {
        auto paths = Path::ProtocolSplit(path);
        auto *backend = GetProcotol(paths.first);
        if (!backend)
            return {};

        return backend->Open(paths.second, mode);
    }

    string GetInternalPath(const string& path)
    {
        return str::Replace(path, "\\", "/");
    }

    string GetNativePath(const string& path)
    {
#ifdef _WIN32
        return str::Replace(path, "/", "\\");
#else
        return pathName;
#endif
    }

    string AddTrailingSlash(const string& path)
    {
        string ret = path;
        str::Trim(ret);
        ret = str::Replace(ret, "\\", "/");
        if (!ret.empty() && ret.back() != '/')
        {
            ret += '/';
        }

        return ret;
    }

    string RemoveTrailingSlash(const string& path)
    {
        string ret = path;
        str::Trim(ret);
        ret = str::Replace(ret, "\\", "/");
        if (!ret.empty() && ret.back() == '/')
        {
            ret.resize(ret.length() - 1);
        }

        return ret;
    }

    void SplitPath(const string& fullPath, string& pathName, string& fileName, string& extension, bool lowerCaseExtension)
    {
        string fullPathCopy = GetInternalPath(fullPath);

        size_t extPos = fullPathCopy.find_last_of('.');
        size_t pathPos = fullPathCopy.find_last_of('/');

        if (extPos != string::npos
            && (pathPos == string::npos || extPos > pathPos))
        {
            extension = fullPathCopy.substr(extPos);
            if (lowerCaseExtension)
            {
                str::ToLower(extension);
            }

            fullPathCopy = fullPathCopy.substr(0, extPos);
        }
        else {
            extension.clear();
        }

        pathPos = fullPathCopy.find_last_of('/');
        if (pathPos != string::npos)
        {
            fileName = fullPathCopy.substr(pathPos + 1);
            pathName = fullPathCopy.substr(0, pathPos + 1);
        }
        else
        {
            fileName = fullPathCopy;
            pathName.clear();
        }
    }

    string GetPath(const string& fullPath)
    {
        string path, file, extension;
        SplitPath(fullPath, path, file, extension);
        return path;
    }

    string GetFileName(const string& fullPath)
    {
        string path, file, extension;
        SplitPath(fullPath, path, file, extension);
        return file;
    }

    string GetExtension(const string& fullPath, bool lowercaseExtension)
    {
        string path, file, extension;
        SplitPath(fullPath, path, file, extension, lowercaseExtension);
        return extension;
    }

    string GetFileNameAndExtension(const string& fileName, bool lowercaseExtension)
    {
        string path, file, extension;
        SplitPath(fileName, path, file, extension, lowercaseExtension);
        return file + extension;
    }

    string GetParentPath(const string& path)
    {
        size_t pos = RemoveTrailingSlash(path).find_last_of('/');
        if (pos != string::npos)
            return path.substr(0, pos + 1);

        return string();
    }

    bool IsAbsolutePath(const string& pathName)
    {
        if (pathName.empty())
            return false;

        string path = GetInternalPath(pathName);

        if (path[0] == '/')
            return true;

#ifdef _WIN32
        if (path.length() > 1 && IsAlpha(path[0]) && path[1] == ':')
            return true;
#endif

        return false;
    }

#ifdef _WIN32
    static std::string ToUniversalPath(const std::string& path)
    {
        return str::Replace(path, "\\", "/");
    }

    string GetCurrentDir()
    {
        wchar_t path[MAX_PATH];
        path[0] = 0;
        GetCurrentDirectoryW(MAX_PATH, path);
        return str::FromWide(path);
    }

    string GetExecutableFolder()
    {
        wchar_t exeName[MAX_PATH];
        exeName[0] = 0;
        GetModuleFileNameW(nullptr, exeName, MAX_PATH);
        return GetPath(ToUniversalPath(str::FromWide(exeName)));
    }

    bool FileExists(const std::string& fileName)
    {
        string fixedName = GetNativePath(RemoveTrailingSlash(fileName));

        DWORD attributes = GetFileAttributesW(str::ToWide(fixedName).c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY)
            return false;

        return true;
    }

    bool DirectoryExists(const string& path)
    {
        string fixedName = GetNativePath(RemoveTrailingSlash(path));

        DWORD attributes = GetFileAttributesW(str::ToWide(fixedName).c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES
            || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            return false;
        }
        return true;
    }

    unique_ptr<Stream> OpenStream(const string &path, StreamMode mode)
    {
        if (mode == StreamMode::ReadOnly
            && !FileExists(path))
        {
            return nullptr;
        }

        try
        {
            unique_ptr<Stream> file(new WindowsFileStream(path, mode));
            return file;
        }
        catch (const std::exception &e)
        {
            ALIMER_LOGERROR("OSFileSystem::Open(): {}", e.what());
            return {};
        }
    }

    void ScanDirInternal(
        vector<string>& result, string path, const string& startPath,
        const string& filter, ScanDirFlags flags, bool recursive)
    {
        path = AddTrailingSlash(path);
        string deltaPath;
        if (path.length() > startPath.length())
            deltaPath = path.substr(startPath.length());

        string filterExtension = filter.substr(filter.find_last_of('.'));
        if (filterExtension.find('*') != string::npos)
            filterExtension.clear();

        WIN32_FIND_DATAW info;
        HANDLE handle = FindFirstFileW(str::ToWide(path + "*").c_str(), &info);
        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                string fileName(str::FromWide(info.cFileName));
                if (!fileName.empty())
                {
                    if (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN && !(flags & ScanDirMask::Hidden))
                        continue;
                    if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if (flags & ScanDirMask::Directories)
                            result.push_back(deltaPath + fileName);
                        if (recursive && fileName != "." && fileName != "..")
                        {
                            ScanDirInternal(result, path + fileName, startPath, filter, flags, recursive);
                        }
                    }
                    else if (flags & ScanDirMask::Files)
                    {
                        if (filterExtension.empty() || str::EndsWith(fileName, filterExtension))
                        {
                            result.push_back(deltaPath + fileName);
                        }
                    }
                }
            } while (FindNextFileW(handle, &info));

            FindClose(handle);
        }
    }

    void ScanDirectory(vector<string>& result, const string& pathName, const string& filter, ScanDirFlags flags, bool recursive)
    {
        result.clear();

        string initialPath = AddTrailingSlash(pathName);
        ScanDirInternal(result, initialPath, initialPath, filter, flags, recursive);
    }

#else
    string GetCurrentDir()
    {
        char path[MAX_PATH];
        path[0] = 0;
        getcwd(path, MAX_PATH);
        return string(path);
    }

    string GetExecutableFolder()
    {
#if defined(__linux__)
        char exeName[MAX_PATH];
        memset(exeName, 0, MAX_PATH);
        pid_t pid = getpid();
        String link = "/proc/" + String(pid) + "/exe";
        readlink(link.CString(), exeName, MAX_PATH);
        return GetPath(string(exeName));
#elif defined(__APPLE__)
        char exeName[MAX_PATH];
        memset(exeName, 0, MAX_PATH);
        unsigned size = MAX_PATH;
        _NSGetExecutablePath(exeName, &size);
        return GetPath(string(exeName));
#else
        return GetCurrentDir();
#endif
    }

    bool FileExists(const std::string& fileName)
    {
        string fixedName = GetNativePath(RemoveTrailingSlash(fileName));

        struct stat st {};
        if (stat(fixedName.c_str(), &st) || st.st_mode & S_IFDIR)
            return false;

        return true;
    }

    bool DirectoryExists(const string& path)
    {
        // Always return true for the root directory
        if (path == "/")
            return true;

        string fixedName = GetNativePath(RemoveTrailingSlash(path));

        struct stat st {};
        if (stat(fixedName.CString(), &st) || !(st.st_mode & S_IFDIR))
            return false;
        return true;
    }


#endif
}
