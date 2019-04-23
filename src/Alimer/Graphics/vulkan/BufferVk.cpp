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

#include "../Buffer.h"
#include "../GraphicsDevice.h"
#include "GraphicsDeviceVk.h"
#include "../../Core/Log.h"

#include <volk.h>
#include <vk_mem_alloc.h>

namespace alimer
{
    bool Buffer::Create(const void* pInitData)
    {
        VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        createInfo.size = _size;
        createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;

        if (any(_usage & BufferUsage::Vertex)) {
            createInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }

        if (any(_usage & BufferUsage::Index)) {
            createInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }

        if (any(_usage & BufferUsage::Uniform)) {
            createInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        if (any(_usage & BufferUsage::Storage)) {
            createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }

        if (any(_usage & BufferUsage::Indirect)) {
            createInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        }

        uint32_t sharing_indices[3];
        const uint32_t graphics_queue_family_index = _device->GetImpl()->GetGraphicsQueueFamily();
        const uint32_t compute_queue_family_index = _device->GetImpl()->GetComputeQueueFamily();
        const uint32_t transfer_queue_family_index = _device->GetImpl()->GetTransferQueueFamily();
        if (graphics_queue_family_index != compute_queue_family_index
            || graphics_queue_family_index != transfer_queue_family_index)
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
        VmaAllocationInfo allocationInfo = {};

        // Determine appropriate memory usage flags.
        VmaAllocationCreateInfo allocCreateInfo = {};
        ResourceUsage resourceUsage = ResourceUsage::Default;
        switch (resourceUsage)
        {
            /*case ResourceUsage::Default:
            case ResourceUsage::Immutable:
                createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                staticBuffer = true;
                break;*/

        case ResourceUsage::Default:
        case ResourceUsage::Immutable:
        case ResourceUsage::Dynamic:
            allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;

        case ResourceUsage::Staging:
            createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            break;

        default:
            break;
        }



        result = vmaCreateBuffer(
            _device->GetImpl()->GetVmaAllocator(),
            &createInfo,
            &allocCreateInfo,
            &_handle,
            &_memory,
            &allocationInfo);

        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("[Vulkan] - Failed to create buffer");
            return false;
        }

        // Handle
        if (pInitData != nullptr)
        {
            memcpy(allocationInfo.pMappedData, pInitData, allocationInfo.size);
            /*if (staticBuffer)
            {
                SetSubDataImpl(0, allocationInfo.size, pInitData);
            }
            else
            {
                void *data;
                vmaMapMemory(_device->GetImpl()->GetVmaAllocator(), _memory, &data);
                memcpy(data, pInitData, allocationInfo.size);
                vmaUnmapMemory(_device->GetImpl()->GetVmaAllocator(), _memory);
            }*/
        }

        return true;
    }

    void Buffer::Destroy()
    {
        if (_memory != VK_NULL_HANDLE) {
            vmaDestroyBuffer(_device->GetImpl()->GetVmaAllocator(), _handle, _memory);
            _memory = VK_NULL_HANDLE;
        }
        else if (_handle != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(_device->GetImpl()->GetVkDevice(), _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }
    }

    bool Buffer::SetSubDataImpl(uint64_t offset, uint64_t size, const void* pData)
    {
        return false;
    }
}
