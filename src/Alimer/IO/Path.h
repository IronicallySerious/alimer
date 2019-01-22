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

#pragma once

#include "../Base/String.h"
#include <utility>

namespace alimer
{
    /// Class for path helpers.
    class ALIMER_API Path
	{
    public:
		static bool IsAbsolutePath(const String &path);
        static bool IsRootPath(const String &path);

        static String Join(const String &base, const String &path);
        static String GetBaseDir(const String &path);
        static String GetBaseName(const String &path);
        static String GetRelativePath(const String &base, const String &path);

        /// Return the extension from a path, converted to lowercase by default.
        static String GetExtension(const String &path, bool lowerCaseExtension = true);

    private:
        Path() = delete;
        Path(const Path&) = delete;
        Path& operator=(const Path&) = delete;
    };
}
