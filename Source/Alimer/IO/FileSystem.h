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

#include "../Core/Flags.h"
#include "../IO/Stream.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Alimer
{
    /// Return files.
    enum class ScanDirMask : uint32_t
    {
        None = 0,
        /// Scan files.
        Files = 0x1,
        /// Scan directories.
        Directories = 0x2,
        /// Scan also hidden files.
        Hidden = 0x4
    };

    using ScanDirFlags = Flags<ScanDirMask, uint32_t>;
    ALIMER_FORCE_INLINE ScanDirFlags operator|(ScanDirMask bit0, ScanDirMask bit1)
    {
        return ScanDirFlags(bit0) | bit1;
    }

    ALIMER_FORCE_INLINE ScanDirFlags operator~(ScanDirMask bits)
    {
        return ~(ScanDirFlags(bits));
    }

    /// Add a slash at the end of the path if missing and convert to internal format (use slashes.)
    ALIMER_API std::string AddTrailingSlash(const std::string& path);
    /// Remove the slash from the end of a path if exists and convert to internal format (use slashes.)
    ALIMER_API std::string RemoveTrailingSlash(const std::string& path);

    /// Split a full path to path, filename and extension. The extension will be converted to lowercase by default.
    ALIMER_API void SplitPath(const std::string& fullPath, std::string& pathName, std::string& fileName, std::string& extension, bool lowerCaseExtension = true);

    /// Return the path from a full path.
    ALIMER_API std::string GetPath(const std::string& fullPath);
    /// Return the filename from a full path.
    ALIMER_API std::string GetFileName(const std::string& fullPath);
    /// Return the extension from a full path, converted to lowercase by default.
    ALIMER_API std::string GetExtension(const std::string& fullPath, bool lowercaseExtension = true);
    /// Return the filename and extension from a full path. The case of the extension is preserved by default, so that the file can be opened in case-sensitive operating systems.
    ALIMER_API std::string GetFileNameAndExtension(const std::string& fileName, bool lowercaseExtension = false);
    /// Return the parent path, or the path itself if not available.
    ALIMER_API std::string GetParentPath(const std::string& path);
    /// Return whether a path is absolute.
    ALIMER_API bool IsAbsolutePath(const std::string& pathName);

    /// Check if a file exists.
    ALIMER_API bool FileExists(const std::string& fileName);
    /// Check if a directory exists.
    ALIMER_API bool DirectoryExists(const std::string& path);
    /// Return the absolute current working directory.
    ALIMER_API std::string GetCurrentDir();
    /// Return the executable application folder.
    ALIMER_API std::string GetExecutableFolder();
    /// Open stream from given path with given access mode.
    ALIMER_API std::unique_ptr<Stream> OpenStream(const std::string &path, StreamMode mode = StreamMode::ReadOnly);
    /// Scan a directory for specified files.
    ALIMER_API void ScanDirectory(std::vector<std::string>& result, const std::string& pathName, const std::string& filter, ScanDirFlags flags, bool recursive);
}
