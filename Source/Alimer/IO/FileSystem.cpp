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
#include "../Core/Log.h"

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#   include "../IO/Windows/WindowsFileSystem.h"
#endif

namespace Alimer
{
    FileSystem::FileSystem()
    {
        // Register default file protocol.
        RegisterProtocol("file", new OSFileSystemProtocol("."));

#ifdef ALIMER_DEFAULT_ASSETS_DIRECTORY
        const char *assetsDir = ALIMER_DEFAULT_ASSETS_DIRECTORY;
        if (assetsDir)
        {
            RegisterProtocol("assets", new OSFileSystemProtocol(assetsDir));
        }
#else
        // Lookup assets folder
        if (DirectoryExists("assets"))
        {
            RegisterProtocol("assets", new OSFileSystemProtocol("assets"));
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

    UniquePtr<Stream> FileSystem::Open(const String &path, StreamMode mode)
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
#ifdef _WIN32
        return path.Replaced('/', '\\');
#else
        return path;
#endif
    }

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

    String GetPath(const String& fullPath)
    {
        String path, file, extension;
        SplitPath(fullPath, path, file, extension);
        return path;
    }

    String GetFileName(const String& fullPath)
    {
        String path, file, extension;
        SplitPath(fullPath, path, file, extension);
        return file;
    }

    String GetExtension(const String& fullPath, bool lowercaseExtension)
    {
        String path, file, extension;
        SplitPath(fullPath, path, file, extension, lowercaseExtension);
        return extension;
    }

    String GetFileNameAndExtension(const String& fileName, bool lowercaseExtension)
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

#ifdef _WIN32
    static String ToUniversalPath(const String& path)
    {
        return path.Replaced("\\", "/");
    }

    String GetCurrentDir()
    {
        wchar_t path[MAX_PATH];
        path[0] = 0;
        GetCurrentDirectoryW(MAX_PATH, path);
        return String(path);
    }

    String GetExecutableFolder()
    {
        wchar_t exeName[MAX_PATH];
        exeName[0] = 0;
        GetModuleFileNameW(nullptr, exeName, MAX_PATH);
        return GetPath(ToUniversalPath(String(exeName)));
    }

    bool FileExists(const String& fileName)
    {
        String fixedName = GetNativePath(RemoveTrailingSlash(fileName));

        DWORD attributes = GetFileAttributesW(WString(fixedName).CString());
        if (attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY)
            return false;

        return true;
    }

    bool DirectoryExists(const String& path)
    {
        String fixedName = GetNativePath(RemoveTrailingSlash(path));

        DWORD attributes = GetFileAttributesW(WString(fixedName).CString());
        if (attributes == INVALID_FILE_ATTRIBUTES
            || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            return false;
        }
        return true;
    }

    UniquePtr<Stream> OpenStream(const String &path, StreamMode mode)
    {
        if (mode == StreamMode::ReadOnly
            && !FileExists(path))
        {
            return nullptr;
        }

        try
        {
            UniquePtr<Stream> file(new WindowsFileStream(path, mode));
            return file;
        }
        catch (const std::exception &e)
        {
            ALIMER_LOGERRORF("OSFileSystem::Open(): %s", e.what());
            return {};
        }
    }

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
