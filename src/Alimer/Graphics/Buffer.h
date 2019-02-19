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

#include <memory>
#include "../Graphics/GPUBackend.h"
#include "../Graphics/VertexFormat.h"
#include "../Graphics/GPUResource.h"

namespace alimer
{
	/// Defines a GPU Buffer class.
	class ALIMER_API Buffer : public GPUResource, public RefCounted
	{
    protected:
        /// Constructor.
        Buffer(GraphicsDevice* device);

    public:
        ~Buffer() override;

        void Destroy() override;

        /// Replace entire buffer data in synchronous way.
        bool SetSubData(const void* pData);

        /// Replace buffer data in synchronous way.
        bool SetSubData(uint32_t offset, uint32_t size, const void* pData);

        /// Get size in bytes of the buffer.
        uint64_t GetSize() const { return _size; }

        /// Get the buffer usage flags.
        BufferUsage GetUsage() const { return _usage; }

        /// Get single element size.
        uint32_t GetStride() const { return _stride; }
        
    private:
        bool Create(const void* pInitData);
        void SetSubDataImpl(uint32_t offset, uint32_t size, const void* pData);

        uint64_t _size = 0;
        BufferUsage _usage = BufferUsage::None;
        uint32_t _stride = 0;

        /// CPU-side shadow data.
        std::unique_ptr<uint8_t[]> _shadowData;

#if defined(ALIMER_VULKAN)
        VkBuffer        _handle = VK_NULL_HANDLE;
        VmaAllocation   _allocation = VK_NULL_HANDLE;
#elif defined(ALIMER_D3D11)
        ID3D11Buffer*   _handle = nullptr;
#endif
	};
}
