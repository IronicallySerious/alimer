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
#include "../Graphics/GpuResource.h"
#include <memory>

namespace Alimer
{
    enum class BufferUsage : uint32_t
    {
        Unknown = 0,
        Vertex = 1 << 0,
        Index = 1 << 1,
        Uniform = 1 << 2,
        Storage = 1 << 3,
        Indirect = 1 << 4,
    };

    enum class IndexType : uint32_t
    {
        UInt16,
        UInt32,
    };

    using BufferUsageFlags = Flags<BufferUsage, uint32_t>;
    ALIMER_FORCE_INLINE BufferUsageFlags operator|(BufferUsage bit0, BufferUsage bit1)
    {
        return BufferUsageFlags(bit0) | bit1;
    }

    ALIMER_FORCE_INLINE BufferUsageFlags operator~(BufferUsage bits)
    {
        return ~(BufferUsageFlags(bits));
    }

    struct GpuBufferDescription
    {
        /// Number of elements in the buffer.
        uint32_t elementCount = 1;
        /// Size of each individual element in the buffer, in bytes. 
        uint32_t elementSize = 0;

        /// Buffer usage.
        BufferUsageFlags usage = BufferUsage::Unknown;

        /// Buffer resource usage.
        ResourceUsage resourceUsage = ResourceUsage::Default;
    };

	/// Defines a GPU Buffer class.
	class GpuBuffer : public GpuResource, public RefCounted
	{
	protected:
		/// Constructor.
		GpuBuffer(const GpuBufferDescription& description);

	public:
		/// Destructor.
		virtual ~GpuBuffer() = default;

        const GpuBufferDescription &GetDescription() const { return _description; }
		inline BufferUsageFlags GetBufferUsage() const { return _description.usage; }
		inline uint32_t GetElementCount() const { return _description.elementCount; }
		inline uint32_t GetElementSize() const { return _description.elementSize; }
		inline uint64_t GetSize() const { return _size; }

	protected:
        GpuBufferDescription _description;
		uint64_t _size;
	};
}
