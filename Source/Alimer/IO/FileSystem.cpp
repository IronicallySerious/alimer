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
#include "../Debug/Log.h"

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#include "../IO/Windows/FileSystemWindows.h"
#endif
using namespace std;

namespace Alimer
{
	FileSystem &FileSystem::Get()
	{
		static FileSystem fileSystem;
		return fileSystem;
	}

	FileSystem::FileSystem()
	{
#ifdef ALIMER_ASSET_PIPELINE
		RegisterProtocol("assets", std::unique_ptr<FileSystemProtocol>(new OSFileSystem("assets")));
#else
		RegisterProtocol("assets", std::unique_ptr<FileSystemProtocol>(new OSFileSystem("assets")));
#endif
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
}
