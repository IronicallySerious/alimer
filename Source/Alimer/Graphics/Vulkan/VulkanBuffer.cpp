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

#include "VulkanGraphicsImpl.h"
#include "VulkanBuffer.h"
#include "VulkanConvert.h"
#include "../../Core/Log.h"

namespace Alimer
{
    VulkanBuffer::VulkanBuffer(VulkanGraphicsDevice* device)
        : _device(device)
    {
    }

    VulkanBuffer::~VulkanBuffer()
    {
        Destroy();
    }

    void VulkanBuffer::Destroy()
    {
        if (_allocation != VK_NULL_HANDLE)
            vmaDestroyBuffer(_device->GetVmaAllocator(), _handle, _allocation);
        else
            vkDestroyBuffer(_device->GetDevice(), _handle, nullptr);
    }

    bool VulkanBuffer::Define(ResourceUsage resourceUsage, BufferUsage bufferUsage, VkDeviceSize size, const void* initialData)
    {
        _bufferUsage = 0;
        if (any(bufferUsage & BufferUsage::Vertex))
            _bufferUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        if (any(bufferUsage & BufferUsage::Index))
            _bufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        if (any(bufferUsage & BufferUsage::Uniform))
            _bufferUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        if (any(bufferUsage & BufferUsage::Storage))
            _bufferUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        if (any(bufferUsage & BufferUsage::Indirect))
            _bufferUsage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

        VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.size = size;
        createInfo.usage = _bufferUsage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        uint32_t sharing_indices[3];
        const uint32_t graphics_queue_family_index = _device->GetGraphicsQueueFamily();
        const uint32_t compute_queue_family_index = _device->GetComputeQueueFamily();
        const uint32_t transfer_queue_family_index = _device->GetTransferQueueFamily();

        if (graphics_queue_family_index != compute_queue_family_index ||
            graphics_queue_family_index != transfer_queue_family_index)
        {
            // For buffers, always just use CONCURRENT access modes,
            // so we don't have to deal with acquire/release barriers in async compute.
            createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

            sharing_indices[createInfo.queueFamilyIndexCount++] = graphics_queue_family_index;

            if (graphics_queue_family_index != compute_queue_family_index)
                sharing_indices[createInfo.queueFamilyIndexCount++] = compute_queue_family_index;

            if (graphics_queue_family_index != transfer_queue_family_index &&
                compute_queue_family_index != transfer_queue_family_index)
            {
                sharing_indices[createInfo.queueFamilyIndexCount++] = transfer_queue_family_index;
            }

            createInfo.pQueueFamilyIndices = sharing_indices;
        }

        // Allocate memory from the Vulkan Memory Allocator.
        VkResult result = VK_SUCCESS;
        const bool noAllocation = false;
        bool staticBuffer = false;
        VmaAllocationInfo allocationInfo = {};
        if (noAllocation)
        {
            result = vkCreateBuffer(_device->GetDevice(), &createInfo, nullptr, &_handle);
        }
        else
        {
            // Determine appropriate memory usage flags.
            VmaAllocationCreateInfo allocCreateInfo = {};
            switch (resourceUsage)
            {
            case ResourceUsage::Default:
            case ResourceUsage::Immutable:
                _bufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                staticBuffer = true;
                break;

            case ResourceUsage::Dynamic:
                allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                break;

            case ResourceUsage::Staging:
                _bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
                allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
                break;

            default:
                break;
            }

            createInfo.usage = _bufferUsage;

            result = vmaCreateBuffer(
                _device->GetVmaAllocator(),
                &createInfo,
                &allocCreateInfo,
                &_handle,
                &_allocation,
                &allocationInfo);
        }

        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("[Vulkan] - Failed to create buffer");
            return false;
        }

        _vkSize = size;

        // Handle
        if (initialData != nullptr)
        {
            if (staticBuffer)
            {
                SetSubData(0, allocationInfo.size, initialData);
            }
            else
            {
                void *data;
                vmaMapMemory(_device->GetVmaAllocator(), _allocation, &data);
                memcpy(data, initialData, allocationInfo.size);
                vmaUnmapMemory(_device->GetVmaAllocator(), _allocation);
            }
        }

        return true;
    }

    bool VulkanBuffer::SetSubData(uint32_t offset, uint32_t size, const void* pData)
    {
        return _device->BufferSubData(this, offset, size, pData) == VK_SUCCESS;
    }

    // VulkanVertexBuffer
    VulkanVertexBuffer::VulkanVertexBuffer(VulkanGraphicsDevice* device, uint32_t vertexCount, size_t elementsCount, const VertexElement* elements, ResourceUsage resourceUsage, const void* initialData)
        : VertexBuffer(device, vertexCount, elementsCount, elements, resourceUsage)
        , VulkanBuffer(device)
    {
        Define(_resourceUsage, _usage, _size, initialData);
    }

    bool VulkanVertexBuffer::SetSubDataImpl(uint32_t offset, uint32_t size, const void* pData)
    {
        return VulkanBuffer::SetSubData(offset, size, pData);
    }
}
