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

#include "../Base/String.h"
#include "../Core/Ptr.h"
#include "../IO/Stream.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Alimer
{
    /// Return files.
    enum class ScanDirFlags : uint32_t
    {
        None = 0,
        /// Scan files.
        Files = 0x1,
        /// Scan directories.
        Directories = 0x2,
        /// Scan also hidden files.
        Hidden = 0x4
    };
    ALIMER_BITMASK(ScanDirFlags);

    /// Add a slash at the end of the path if missing and convert to internal format (use slashes.)
    ALIMER_API String AddTrailingSlash(const String& path);
    /// Remove the slash from the end of a path if exists and convert to internal format (use slashes.)
    ALIMER_API String RemoveTrailingSlash(const String& path);

    /// Split a full path to path, filename and extension. The extension will be converted to lowercase by default.
    ALIMER_API void SplitPath(const String& fullPath, String& pathName, String& fileName, String& extension, bool lowerCaseExtension = true);

    /// Return the parent path, or the path itself if not available.
    ALIMER_API String GetParentPath(const String& path);
    /// Return whether a path is absolute.
    ALIMER_API bool IsAbsolutePath(const String& pathName);

    /// Check if a file exists.
    ALIMER_API bool FileExists(const String& fileName);
    /// Check if a directory exists.
    ALIMER_API bool DirectoryExists(const String& path);
    /// Return the absolute current working directory.
    ALIMER_API String GetCurrentDir();
    /// Return the executable application folder.
    ALIMER_API String GetExecutableFolder();
    /// Open stream from given path with given access mode.
    ALIMER_API UniquePtr<Stream> OpenStream(const String &path, StreamMode mode = StreamMode::ReadOnly);
    /// Scan a directory for specified files.
    ALIMER_API void ScanDirectory(std::vector<String>& result, const String& pathName, const String& filter, ScanDirFlags flags, bool recursive);

    /// Backend protocol for file system.
    class ALIMER_API FileSystemProtocol
    {
    public:
        virtual ~FileSystemProtocol() = default;

        virtual bool Exists(const String &path) = 0;
        virtual UniquePtr<Stream> Open(const String &path, StreamMode mode = StreamMode::ReadOnly) = 0;

        inline virtual String GetFileSystemPath(const String&)
        {
            return "";
        }

        /// Gets the name.
        String GetName() const { return _name; }

        /// Set the name.
        void SetName(const String &name) { _name = name; }

    protected:
        String _name;
    };

    /// Class for accessing File system.
    class ALIMER_API FileSystem
    {
    public:
        /// Returns the FileSystem instance.
        static FileSystem &Get();

        /// Return the path from a full path.
        static String GetPath(const String& fullPath);
        /// Return the filename from a full path.
        static String GetFileName(const String& fullPath);

        /// Return the extension from a full path, converted to lowercase by default.
        static String GetExtension(const String& fullPath, bool lowercaseExtension = true);

        /// Return the filename and extension from a full path. The case of the extension is preserved by default, so that the file can be opened in case-sensitive operating systems.
        static String GetFileNameAndExtension(const String& fileName, bool lowercaseExtension = false);

        /// Register protocol with given name.
        void RegisterProtocol(const String &name, FileSystemProtocol* protocol);

        /// Get protocol by name or empty for default protocol.
        FileSystemProtocol* GetProcotol(const String &name);

        /// Check if file exists.
        bool Exists(const String &path);

        /// Open stream from given path with given access mode.
        UniquePtr<Stream> Open(const String &path, StreamMode mode = StreamMode::ReadOnly);

    private:
        FileSystem();

        std::unordered_map<String, UniquePtr<FileSystemProtocol>> _protocols;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(FileSystem);
    };
}
