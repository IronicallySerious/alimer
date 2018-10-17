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

#include "../GpuBuffer.h"
#include "VulkanBackend.h"
#include <vector>

namespace Alimer
{
    class VulkanGraphicsDevice;

	/// Vulkan Buffer.
	class VulkanBuffer 
	{
	public:
        VulkanBuffer(VulkanGraphicsDevice* device);
        ~VulkanBuffer();
        void Destroy();

        bool Define(ResourceUsage resourceUsage, BufferUsage bufferUsage, VkDeviceSize size, const void* initialData);
        bool SetSubData(uint32_t offset, uint32_t size, const void* pData);

		VkBuffer GetHandle() const { return _handle; }
        VmaAllocation GetAllocation() const { return _allocation; }
        VkDeviceSize GetSize() const { return _vkSize; }
	private:
        VulkanGraphicsDevice* _device;
        VkBuffer _handle = VK_NULL_HANDLE;
        VmaAllocation _allocation = VK_NULL_HANDLE;
        VkBufferUsageFlags _bufferUsage = 0;
        VkDeviceSize _vkSize = 0;
	};

    class VulkanVertexBuffer final : public VertexBuffer, public VulkanBuffer
    {
    public:
        VulkanVertexBuffer(VulkanGraphicsDevice* device, uint32_t vertexCount, size_t elementsCount, const VertexElement* elements, ResourceUsage resourceUsage, const void* initialData);

        bool SetSubDataImpl(uint32_t offset, uint32_t size, const void* pData) override;
    };

    class VulkanIndexBuffer final : public IndexBuffer, public VulkanBuffer
    {
    public:
        VulkanIndexBuffer(VulkanGraphicsDevice* device, uint32_t indexCount, IndexType indexType, ResourceUsage resourceUsage, const void* initialData);

        bool SetSubDataImpl(uint32_t offset, uint32_t size, const void* pData) override;
    };
}
