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

#include "../Base/Ptr.h"
#include "../Base/String.h"
#include "../Graphics/agpu.h"

namespace Alimer
{
    enum class BufferUsage : uint32_t
    {
        None = AGPU_BUFFER_USAGE_NONE,
        Vertex = AGPU_BUFFER_USAGE_VERTEX,
        Index = AGPU_BUFFER_USAGE_INDEX,
        Uniform = AGPU_BUFFER_USAGE_UNIFORM,
        Storage = AGPU_BUFFER_USAGE_STORAGE,
        Indirect = AGPU_BUFFER_USAGE_INDIRECT,
        Dynamic = AGPU_BUFFER_USAGE_DYNAMIC,
        CPUAccessible = AGPU_BUFFER_USAGE_CPU_ACCESSIBLE,
    };
    ALIMER_BITMASK(BufferUsage);

	/// Defines a Graphics Buffer class.
	class ALIMER_API GpuBuffer final : public RefCounted
	{
	public:
        /// Constructor.
        GpuBuffer();

        /// Destructor.
        ~GpuBuffer();

        /// Unconditionally destroy the GPU resource.
        void Destroy();

        bool Define(BufferUsage usage, uint64_t elementCount, uint64_t stride, void* initialData = nullptr, const String& name = "");

        /// Replace entire buffer data in synchronous way.
        bool SetSubData(const void* pData);

        /// Replace buffer data in synchronous way.
        bool SetSubData(uint32_t offset, uint32_t size, const void* pData);

        /// Get the gpu handle.
        AgpuBuffer GetHandle() const { return _handle; }

        /// Get the buffer usage flags.
        BufferUsage GetUsage() const { return _usage; }

        /// Get size in bytes of the buffer.
        uint64_t GetSize() const { return _size; }

        /// Get single element size in bytes.
		uint32_t GetStride() const { return _stride; }

	private:
        AgpuBuffer _handle = nullptr;
        BufferUsage _usage = BufferUsage::None;
        uint64_t _elementCount = 0;
        uint64_t _stride = 0;
		uint64_t _size = 0;
	};
}
