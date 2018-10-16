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
#include "VulkanPipelineLayout.h"
#include "VulkanConvert.h"
#include "../../Core/Log.h"

#if TODO
namespace Alimer
{
    VulkanDescriptorSetAllocator::VulkanDescriptorSetAllocator(VulkanGraphics* graphics, const DescriptorSetLayout &layout)
        : _logicalDevice(graphics->GetLogicalDevice())
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (uint32_t i = 0; i < MaxBindingsPerSet; i++)
        {
            uint32_t types = 0;
            if (layout.uniformBufferMask & (1u << i))
            {
                bindings.push_back({ i, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, layout.stages, nullptr });
                _poolSize.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VulkanSetsCountPerPool });
                types++;
            }

            (void)types;
            ALIMER_ASSERT(types <= 1 && "Descriptor set aliasing!");
        }

        VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        if (!bindings.empty())
        {
            createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            createInfo.pBindings = bindings.data();
        }

        if (vkCreateDescriptorSetLayout(_logicalDevice, &createInfo, nullptr, &_vkHandle) != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan - Failed to create DescriptorSetLayout.");
        }
    }

    VulkanDescriptorSetAllocator::~VulkanDescriptorSetAllocator()
    {
        if (_vkHandle != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(_logicalDevice, _vkHandle, nullptr);
            _vkHandle = VK_NULL_HANDLE;
        }
        Clear();
    }

    void VulkanDescriptorSetAllocator::Clear()
    {
        _setNodes.Clear();
        for (auto &pool : _pools)
        {
            vkResetDescriptorPool(_logicalDevice, pool, 0);
            vkDestroyDescriptorPool(_logicalDevice, pool, nullptr);
        }
        _pools.clear();
    }

    void VulkanDescriptorSetAllocator::BeginFrame()
    {
        _setNodes.BeginFrame();
    }

    std::pair<VkDescriptorSet, bool> VulkanDescriptorSetAllocator::Find(uint64_t hash)
    {
        DescriptorSetNode *node = _setNodes.Request(hash);
        if (node)
            return { node->set, true };

        node = _setNodes.RequestVacant(hash);
        if (node)
            return { node->set, false };

        VkDescriptorPool pool;
        VkDescriptorPoolCreateInfo poolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        poolCreateInfo.maxSets = VulkanSetsCountPerPool;
        if (!_poolSize.empty())
        {
            poolCreateInfo.poolSizeCount = static_cast<uint32_t>(_poolSize.size());
            poolCreateInfo.pPoolSizes = _poolSize.data();
        }

        if (vkCreateDescriptorPool(_logicalDevice, &poolCreateInfo, nullptr, &pool) != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan - Failed to create DescriptorPool.");
        }

        VkDescriptorSet sets[VulkanSetsCountPerPool];
        VkDescriptorSetLayout layouts[VulkanSetsCountPerPool];
        std::fill(std::begin(layouts), std::end(layouts), _vkHandle);

        VkDescriptorSetAllocateInfo alloc = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        alloc.descriptorPool = pool;
        alloc.descriptorSetCount = VulkanSetsCountPerPool;
        alloc.pSetLayouts = layouts;

        if (vkAllocateDescriptorSets(_logicalDevice, &alloc, sets) != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan - Failed to allocate descriptor sets.");
        }
        _pools.push_back(pool);

        for (VkDescriptorSet set : sets)
        {
            _setNodes.MakeVacant(set);
        }

        return { _setNodes.RequestVacant(hash)->set, false };
    }

    VulkanPipelineLayout::VulkanPipelineLayout(VulkanGraphics* graphics, const ResourceLayout &layout)
        : _logicalDevice(graphics->GetLogicalDevice())
        , _layout(layout)
    {
        VkDescriptorSetLayout layouts[MaxDescriptorSets] = {};
        uint32_t numSets = 0;
        for (uint32_t i = 0; i < MaxDescriptorSets; i++)
        {
            if (!layout.sets[i].stages)
                continue;

            _setAllocators[i] = graphics->RequestDescriptorSetAllocator(layout.sets[i]);
            layouts[i] = _setAllocators[i]->GetVkHandle();
            if (layout.descriptorSetMask & (1u << i))
            {
                numSets = i + 1;
            }
        }

        VkPipelineLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        if (numSets)
        {
            createInfo.setLayoutCount = numSets;
            createInfo.pSetLayouts = layouts;
        }

        if (vkCreatePipelineLayout(_logicalDevice, &createInfo, nullptr, &_vkHandle) != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("[Vulkan] - Failed to create pipeline layout.");
        }
    }

    VulkanPipelineLayout::~VulkanPipelineLayout()
    {
        if (_vkHandle != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(_logicalDevice, _vkHandle, nullptr);
            _vkHandle = VK_NULL_HANDLE;
        }
    }
}

#endif // TODO
