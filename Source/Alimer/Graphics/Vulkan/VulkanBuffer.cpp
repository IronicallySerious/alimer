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
    VulkanBuffer::VulkanBuffer(VulkanGraphics* graphics, BufferUsageFlags usage, uint64_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData)
        : _logicalDevice(graphics->GetLogicalDevice())
        , _allocator(graphics->GetAllocator())
    {
        VkBufferUsageFlags vkUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        if (resourceUsage == ResourceUsage::Dynamic)
        {
            vkUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        if (usage & BufferUsage::Vertex)
            vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        if (usage & BufferUsage::Index)
            vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        if (usage & BufferUsage::Uniform)
            vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        if (usage & BufferUsage::Storage)
            vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        if (usage & BufferUsage::Indirect)
            vkUsage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

        VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.size = size;
        createInfo.usage = vkUsage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;

        // TODO:
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo vmaAllocInfo = {};
        VkResult result = vmaCreateBuffer(
            _allocator,
            &createInfo,
            &allocInfo,
            &_vkHandle,
            &_allocation,
            &vmaAllocInfo);

        if (initialData != nullptr)
        {
            memcpy(vmaAllocInfo.pMappedData, initialData, vmaAllocInfo.size);
        }
    }

    VulkanBuffer::~VulkanBuffer()
    {
        vmaDestroyBuffer(_allocator, _vkHandle, _allocation);
    }

    bool VulkanBuffer::SetData(uint32_t offset, uint32_t size, const void* data)
    {
        ALIMER_LOGCRITICAL("VulkanBuffer::SetData not implemented");
        //memcpy(_allocation->GetMappedData(), initialData, vmaAllocInfo.size);
        return false;
    }
}
