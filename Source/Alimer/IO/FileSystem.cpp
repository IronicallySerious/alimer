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
#include "../Core/Log.h"

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#include "../IO/Windows/FileSystemWindows.h"
#endif
using namespace std;

namespace Alimer
{
    string RemoveTrailingSlash(const string& pathName)
    {
        string ret = pathName;
        Util::Trim(ret);
        ret = Util::Replace(ret, "\\", "/");
        if (!ret.empty() && ret.back() == '/')
        {
            ret.resize(ret.length() - 1);
        }

        return ret;
    }

    string GetParentPath(const string& path)
    {
        size_t pos = RemoveTrailingSlash(path).find_last_of('/');
        if (pos != string::npos)
            return path.substr(0, pos + 1);

        return string();
    }

    FileSystem &FileSystem::Get()
    {
        static FileSystem fileSystem;
        return fileSystem;
    }

    FileSystem::FileSystem()
    {
        std::string parentAssetsDirectory = Path::Join(GetParentPath(FileSystem::GetExecutableFolder()), "assets");
        //if (DirectoryExists(parentAssetsDirectory))
            RegisterProtocol("assets", std::unique_ptr<FileSystemProtocol>(new OSFileSystem(parentAssetsDirectory)));
        //else
        //    RegisterProtocol("assets", std::unique_ptr<FileSystemProtocol>(new OSFileSystem("assets")));
    }

    void FileSystem::RegisterProtocol(const std::string &proto, std::unique_ptr<FileSystemProtocol> protocol)
    {
        protocol->SetProtocol(proto);
        _protocols[proto] = std::move(protocol);
    }

    FileSystemProtocol *FileSystem::GetProtocol(const std::string &proto) const
    {
        auto itr = _protocols.find(proto);
        if (itr != end(_protocols))
            return itr->second.get();

        // Not found.
        return nullptr;
    }

    bool FileSystem::FileExists(const std::string& path) const
    {
        auto paths = Path::ProtocolSplit(path);
        auto *backend = GetProtocol(paths.first);
        if (!backend)
            return false;

        return backend->FileExists(paths.second);
    }

    bool FileSystem::DirectoryExists(const std::string& path) const
    {
        auto paths = Path::ProtocolSplit(path);
        auto *backend = GetProtocol(paths.first);
        if (!backend)
            return false;

        return backend->DirectoryExists(paths.second);
    }

    std::unique_ptr<Stream> FileSystem::Open(const std::string &path, StreamMode mode)
    {
        auto paths = Path::ProtocolSplit(path);
        auto *backend = GetProtocol(paths.first);
        if (!backend)
            return {};

        return backend->Open(paths.second, mode);
    }

    std::string FileSystem::ReadAllText(const std::string &path)
    {
        auto paths = Path::ProtocolSplit(path);
        auto *backend = GetProtocol(paths.first);
        if (!backend)
            return {};

        auto stream = backend->Open(paths.second, StreamMode::ReadOnly);
        return stream->ReadAllText();
    }

    std::vector<uint8_t> FileSystem::ReadAllBytes(const std::string& path)
    {
        auto paths = Path::ProtocolSplit(path);
        auto *backend = GetProtocol(paths.first);
        if (!backend)
            return {};

        auto stream = backend->Open(paths.second, StreamMode::ReadOnly);
        return stream->ReadBytes();
    }
}
