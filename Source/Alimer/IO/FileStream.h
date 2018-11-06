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

namespace Alimer
{
	enum class FileAccess
	{
        ReadOnly,
		WriteOnly,
		ReadWrite
	};

	/// OS file stream.
	class ALIMER_API FileStream : public Stream
	{
	public:
		/// Constructor.
        FileStream();

        /// Construct and open a file.
        FileStream(const String& fileName, FileAccess mode = FileAccess::ReadOnly);

        /// Destructor. Close the file if open.
        ~FileStream() override;

        /// Open a file. Return true on success.
        bool Open(const String& fileName, FileAccess mode = FileAccess::ReadOnly);

        /// Close the file.
        void Close();

        /// Flush any buffered output to the file.
        void Flush();

        bool CanRead() const override;
        bool CanWrite() const override;
        bool CanSeek() const override;

		size_t Read(void* dest, size_t size) override;
        size_t Write(const void* data, size_t size) override;

        /// Return whether is open.
        bool IsOpen() const;

        /// Return the file handle.
        void* GetHandle() const { return _handle; }

    private:
        FileAccess _mode;
        void* _handle;
        bool _canSeek;
	};
}
