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

#include "../../PlatformIncl.h"
#include "../FileSystem.h"

namespace Alimer
{
    class WindowsFileStream final : public Stream
    {
    public:
        WindowsFileStream(const std::string &path, StreamMode mode);
        ~WindowsFileStream() override;

        bool CanSeek() const override { return _handle != nullptr; }

        size_t Read(void* dest, size_t size) override;
        void Write(const void* data, size_t size) override;

    private:
        HANDLE _handle = nullptr;
    };

    /// OS file system protocol protocol for file system.
    class OSFileSystemProtocol final : public FileSystemProtocol
    {
    public:
        OSFileSystemProtocol(const std::string &rootDirectory);
        ~OSFileSystemProtocol();

        std::string GetFileSystemPath(const std::string& path) override;

        std::unique_ptr<Stream> Open(const std::string &path, StreamMode mode) override;

    protected:
        std::string _rootDirectory;
    };
}
