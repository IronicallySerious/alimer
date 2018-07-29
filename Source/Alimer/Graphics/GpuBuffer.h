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

    class BufferHandle;

	/// Defines a GPU Buffer class.
	class GpuBuffer : public GpuResource, public RefCounted
	{
	protected:
        /// Constructor.
        GpuBuffer(Graphics* graphics, BufferUsageFlags usage, ResourceUsage resourceUsage);

    public:
		/// Constructor.
		GpuBuffer(Graphics* graphics, const GpuBufferDescription& description, const void* initialData = nullptr);

	public:
		/// Destructor.
		virtual ~GpuBuffer();

        void Destroy() override;

        /// Get the buffer usage flags.
		BufferUsageFlags GetUsage() const { return _usage; }

        /// Get single element size in bytes.
		uint32_t GetStride() const { return _stride; }

        /// Get size in bytes of the buffer.
		uint64_t GetSize() const { return _size; }

        /// Get the backend buffer implementation.
        BufferHandle* GetHandle() { return _handle; }

    protected:
        bool Create(const void* initialData);
        bool SetData(uint32_t offset, uint32_t size, const void* data);

	protected:
        BufferUsageFlags _usage = BufferUsage::Unknown;
		uint64_t _size = 0;
        uint32_t _stride = 0;

        /// Backend handle.
        BufferHandle* _handle = nullptr;
	};
}
