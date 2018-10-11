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

#include "../../Graphics/Graphics.h"
#include "VulkanBuffer.h"
#include "VulkanConvert.h"
#include "../../Core/Log.h"

namespace Alimer
{
    VulkanBuffer::VulkanBuffer(Graphics* graphics, const BufferDescriptor* descriptor, const void* initialData)
        : GpuBuffer(nullptr, descriptor)
        , _logicalDevice(graphics->GetImpl()->GetDevice())
    {
        VkBufferUsageFlags vkUsage = 0;

        if (any(descriptor->usage & BufferUsage::Vertex))
            vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        if (any(descriptor->usage & BufferUsage::Index))
            vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        if (any(descriptor->usage & BufferUsage::Uniform))
            vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        if (any(descriptor->usage & BufferUsage::Storage))
            vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        if (any(descriptor->usage & BufferUsage::Indirect))
            vkUsage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

        VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.size = descriptor->size;
        createInfo.usage = vkUsage;

        // TODO: Handle queue and sharing.
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;

        // Allocate memory from the Vulkan Memory Allocator.
        VkResult result = VK_SUCCESS;
        const bool noAllocation = false;
        bool staticBuffer = false;
        VmaAllocationInfo allocationInfo = {};
        if (noAllocation)
        {
            result = vkCreateBuffer(_logicalDevice, &createInfo, nullptr, &_handle);
        }
        else
        {
            // Determine appropriate memory usage flags.
            VmaAllocationCreateInfo allocCreateInfo = {};
            switch (descriptor->resourceUsage)
            {
            case ResourceUsage::Default:
            case ResourceUsage::Immutable:
                vkUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                staticBuffer = true;
                break;

            case ResourceUsage::Dynamic:
                allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                break;

            case ResourceUsage::Staging:
                vkUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
                break;

            default:
                break;
            }

            result = vmaCreateBuffer(
                _graphics->GetImpl()->GetVmaAllocator(),
                &createInfo,
                &allocCreateInfo,
                &_handle,
                &_allocation,
                &allocationInfo);
        }

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
                vmaMapMemory(_graphics->GetImpl()->GetVmaAllocator(), _allocation, &data);
                memcpy(data, initialData, allocationInfo.size);
                vmaUnmapMemory(_graphics->GetImpl()->GetVmaAllocator(), _allocation);
            }
        }
    }

    VulkanBuffer::~VulkanBuffer()
    {
        if (_allocation != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(_graphics->GetImpl()->GetVmaAllocator(), _handle, _allocation);
        }
        else
        {
            vkDestroyBuffer(_logicalDevice, _handle, nullptr);
        }
    }

    bool VulkanBuffer::SetSubDataImpl(uint32_t offset, uint32_t size, const void* pData)
    {
        ALIMER_UNUSED(offset);
        ALIMER_UNUSED(size);
        ALIMER_UNUSED(pData);
        ALIMER_LOGCRITICAL("VulkanBuffer::SetData not implemented");
        //memcpy(_allocation->GetMappedData(), initialData, vmaAllocInfo.size);
    }
}
