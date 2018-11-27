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
#include <vector>

namespace Alimer
{
	/// A MemoryStream represents a Stream in memory.
	class ALIMER_API MemoryStream : public Stream
	{
	public:
        /// Construct with a pointer and size.
        MemoryStream(void* data, size_t sizeInBytes);
        /// Construct as read-only with a pointer and size.
        MemoryStream(const void* data, size_t sizeInBytes);
        /// Construct from a vector, which must not go out of scope before MemoryBuffer.
        MemoryStream(std::vector<uint8_t>& data);
        /// Construct from a read-only vector, which must not go out of scope before MemoryBuffer.
        MemoryStream(const std::vector<uint8_t>& data);

        bool CanRead() const override;
        bool CanWrite() const override;
        bool CanSeek() const override;

        uint64_t Read(void* dest, uint64_t size) override;
        void Write(const void* data, uint64_t size) override;

        /// Return memory area.
        uint8_t* Data() { return _buffer; }

    private:
        /// Pointer to the memory area.
        uint8_t* _buffer;
        /// Read-only flag.
        bool _readOnly;
	};
}
