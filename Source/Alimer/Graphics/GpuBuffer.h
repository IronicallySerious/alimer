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

#include "../Graphics/GpuResource.h"

namespace Alimer
{
	/// Defines a GPU Buffer class.
	class GpuBuffer : public GpuResource, public RefCounted
	{
	protected:
		/// Constructor.
		GpuBuffer(Graphics* graphics, MemoryFlags memoryFlags, const BufferDescriptor* descriptor);

	public:
		/// Destructor.
		virtual ~GpuBuffer();

        /// Replace entire buffer data in synchronous way.
        bool SetSubData(const void* pData);

        /// Replace buffer data in synchronous way.
        bool SetSubData(uint32_t offset, uint32_t size, const void* pData);

        /// Get the buffer memory flags.
        MemoryFlags GetMemoryFlags() const { return _memoryFlags; }

        /// Get the buffer usage flags.
        BufferUsageFlags GetUsage() const { return _usage; }

        /// Get size in bytes of the buffer.
        uint64_t GetSize() const { return _size; }

        /// Get single element size in bytes.
		uint32_t GetStride() const { return _stride; }

	private:
        virtual bool SetSubDataImpl(uint32_t offset, uint32_t size, const void* pData) = 0;

        MemoryFlags _memoryFlags;
        BufferUsageFlags _usage;
		uint64_t _size;
        uint32_t _stride;
	};
}
