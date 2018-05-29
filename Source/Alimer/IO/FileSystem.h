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

#pragma once

#include "../IO/Stream.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Alimer
{
	class FileSystemProtocol
	{
	public:
		virtual ~FileSystemProtocol() = default;

		virtual bool FileExists(const std::string &path) = 0;
		virtual bool DirectoryExists(const std::string &path) = 0;

		virtual std::unique_ptr<Stream> Open(const std::string &path, StreamMode mode = StreamMode::ReadOnly) = 0;

		/// Return optional resolve full path.
		const std::string& GetPath() const { return _path; }

		void SetProtocol(const std::string &proto)
		{
			_protocol = proto;
		}

	protected:
		std::string _protocol;
		std::string _path;
	};

	/// FileSystem subsystem. 
	class FileSystem final
	{
	public:
		static FileSystem &Get();

		/// Register file system protocol.
		void RegisterProtocol(const std::string &proto, std::unique_ptr<FileSystemProtocol> protocol);

		/// Get a registered protocol
		FileSystemProtocol *GetProtocol(const std::string &proto) const;

		/// Check if a file exists.
		bool FileExists(const std::string& path) const;

		/// Check if a directory exists.
		bool DirectoryExists(const std::string& path) const;

		std::unique_ptr<Stream> Open(const std::string &path, StreamMode mode = StreamMode::ReadOnly);

		/// Read entire file as text.
		std::string ReadAllText(const std::string &path);

		/// Read entire file as vector bytes.
		std::vector<uint8_t> ReadAllBytes(const std::string& path);
	private:
		/// Constructor.
		FileSystem();

		std::unordered_map<std::string, std::unique_ptr<FileSystemProtocol>> _protocols;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(FileSystem);
	};
}
