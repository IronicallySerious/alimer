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

#include "VulkanBuffer.h"
#include "VulkanGraphics.h"
#include "VulkanConvert.h"
#include "../../Core/Log.h"

namespace Alimer
{
    static std::unordered_map<MemoryFlags, VmaMemoryUsage> memoryFlagsToVmaMemoryUsage = {
        { MemoryFlags::GpuOnly, VMA_MEMORY_USAGE_GPU_ONLY },
        { MemoryFlags::CpuOnly, VMA_MEMORY_USAGE_CPU_ONLY },
        { MemoryFlags::CpuToGpu, VMA_MEMORY_USAGE_CPU_TO_GPU },
        { MemoryFlags::GpuToCpu, VMA_MEMORY_USAGE_GPU_TO_CPU },
    };

    static std::unordered_map<MemoryFlags, VmaAllocationCreateFlagBits> memoryFlagsToVmaMemoryCreateFlag = {
        { MemoryFlags(0), static_cast<VmaAllocationCreateFlagBits>(0) },
        { MemoryFlags::DedicatedAllocation , VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT }
    };

    VulkanBuffer::VulkanBuffer(VulkanGraphics* graphics, MemoryFlags memoryFlags, const BufferDescriptor* descriptor, const void* initialData)
        : GpuBuffer(graphics, memoryFlags, descriptor)
        , _logicalDevice(graphics->GetLogicalDevice())
        , _allocator(graphics->GetAllocator())
    {
        VkBufferUsageFlags vkUsage = 0;

        if (descriptor->usage & BufferUsage::TransferSrc)
            vkUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        if (descriptor->usage & BufferUsage::TransferDest)
            vkUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        if (descriptor->usage & BufferUsage::Vertex)
            vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        if (descriptor->usage & BufferUsage::Index)
            vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        if (descriptor->usage & BufferUsage::Uniform)
            vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        if (descriptor->usage & BufferUsage::Storage)
            vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        if (descriptor->usage & BufferUsage::Indirect)
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
        if (static_cast<uint32_t>(memoryFlags & MemoryFlags::NoAllocation) == 0)
        {
            // Determine appropriate memory usage flags.
            VmaAllocationCreateInfo allocCreateInfo = {};
            MemoryFlags flagsNoDedicated = MemoryFlags(static_cast<uint32_t>(MemoryFlags::DedicatedAllocation) - 1U);
            allocCreateInfo.usage = memoryFlagsToVmaMemoryUsage.at(memoryFlags & flagsNoDedicated);
            allocCreateInfo.flags = memoryFlagsToVmaMemoryCreateFlag.at(memoryFlags & ~flagsNoDedicated);

            result = vmaCreateBuffer(_allocator, &createInfo, &allocCreateInfo, &_handle, &_allocation, &_allocationInfo);
        }
        else
        {
            result = vkCreateBuffer(_logicalDevice, &createInfo, nullptr, &_handle);
        }

        // Handle
        if (initialData != nullptr)
        {
            if (any(memoryFlags & MemoryFlags::GpuOnly))
            {
                SetSubData(0, _allocationInfo.size, initialData);
            }
            else
            {
                void *data;
                vmaMapMemory(_allocator, _allocation, &data);
                memcpy(data, initialData, _allocationInfo.size);
                vmaUnmapMemory(_allocator, _allocation);
            }
        }
    }

    VulkanBuffer::~VulkanBuffer()
    {
        if (_allocation != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(_allocator, _handle, _allocation);
        }
        else
        {
            vkDestroyBuffer(_logicalDevice, _handle, nullptr);
        }
    }

    bool VulkanBuffer::SetSubDataImpl(uint32_t offset, uint32_t size, const void* pData)
    {
        ALIMER_LOGCRITICAL("VulkanBuffer::SetData not implemented");
        //memcpy(_allocation->GetMappedData(), initialData, vmaAllocInfo.size);
    }
}
