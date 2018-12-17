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
#include "../Graphics/GraphicsResource.h"

namespace Alimer
{
    enum class BufferUsage : uint32_t
    {
        None = 0,
        Vertex = 1 << 0,
        Index = 1 << 1,
        Uniform = 1 << 2,
        Storage = 1 << 3,
        Indirect = 1 << 4,
        Dynamic = 1 << 5,
        CPUAccessible = 1 << 6,
    };
    ALIMER_BITMASK(BufferUsage);

    struct BufferDescriptor
    {
        BufferUsage usage = BufferUsage::None;
        uint64_t size = 0;
        uint32_t stride = 0;
        String name;
    };

	/// Defines a Graphics Buffer class.
	class ALIMER_API GpuBuffer : public GraphicsResource
	{
	public:
        /// Constructor.
        GpuBuffer(GPUDevice* device);

        bool Define(const BufferDescriptor* descriptor, const void* initialData = nullptr);

        /// Replace entire buffer data in synchronous way.
        bool SetSubData(const void* pData);

        /// Replace buffer data in synchronous way.
        bool SetSubData(uint32_t offset, uint32_t size, const void* pData);

        /// Get the buffer usage flags.
        BufferUsage GetUsage() const { return _usage; }

        /// Get size in bytes of the buffer.
        uint64_t GetSize() const { return _size; }

    protected:
        virtual bool Create(const void* initialData) = 0;

        BufferUsage _usage = BufferUsage::None;
        uint32_t _stride = 0;
		uint64_t _size = 0;
	};
}
