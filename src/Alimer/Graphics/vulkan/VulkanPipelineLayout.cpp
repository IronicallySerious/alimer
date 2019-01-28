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

#include "VulkanPipelineLayout.h"
#include "GPUDeviceVk.h"
#include "BackendVk.h"
#include "../../Core/Log.h"

namespace alimer
{
    VulkanPipelineLayout::VulkanPipelineLayout(GPUDeviceVk* device, uint64_t hash, const VulkanResourceLayout* layout)
        : _device(device)
        , _hash(hash)
        , _layout(*layout)
    {
        VkDescriptorSetLayout layouts[MaxDescriptorSets] = {};
        uint32_t numSets = 0;
        /*for (uint32_t i = 0; i < MaxDescriptorSets; i++)
        {
            if (!layout.sets[i].stages)
                continue;

            _setAllocators[i] = graphics->RequestDescriptorSetAllocator(layout.sets[i]);
            layouts[i] = _setAllocators[i]->GetVkHandle();
            if (layout.descriptorSetMask & (1u << i))
            {
                numSets = i + 1;
            }
        }*/

        VkPipelineLayoutCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        if (numSets)
        {
            createInfo.setLayoutCount = numSets;
            createInfo.pSetLayouts = layouts;
        }

        if (vkCreatePipelineLayout(_device->GetVkDevice(), &createInfo, nullptr, &_handle) != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("[Vulkan] - Failed to create pipeline layout.");
        }
    }

    VulkanPipelineLayout::~VulkanPipelineLayout()
    {
        if (_handle != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(_device->GetVkDevice(), _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }
    }
}
