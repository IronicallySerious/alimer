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

#include "../Shader.h"
#include "VulkanPrerequisites.h"
#include "../../Util/HashMap.h"
#include "../../Util/ObjectPool.h"
#include "../../Util/TemporaryHashmap.h"
#include <vector>

namespace Alimer
{
    class VulkanGraphics;

    static constexpr unsigned VulkanSetsCountPerPool = 16;
    static constexpr unsigned VulkanDescriptorRingSize = 8;

    class VulkanDescriptorSetAllocator final
    {
    public:
        VulkanDescriptorSetAllocator(VulkanGraphics* graphics, const DescriptorSetLayout &layout);
        ~VulkanDescriptorSetAllocator();

        void Clear();
        void BeginFrame();
        std::pair<VkDescriptorSet, bool> Find(uint64_t hash);

        VkDescriptorSetLayout GetVkHandle() const { return _vkHandle; }

    private:
        struct DescriptorSetNode : TemporaryHashmapEnabled<DescriptorSetNode>, IntrusiveListEnabled<DescriptorSetNode>
        {
            DescriptorSetNode(VkDescriptorSet set)
                : set(set)
            {
            }

            VkDescriptorSet set;
        };

        VkDevice _logicalDevice;
        VkDescriptorSetLayout _vkHandle = VK_NULL_HANDLE;
        std::vector<VkDescriptorPoolSize> _poolSize;
        std::vector<VkDescriptorPool> _pools;
        TemporaryHashmap<DescriptorSetNode, VulkanDescriptorRingSize, true> _setNodes;
    };

    class VulkanPipelineLayout final
    {
    public:
        VulkanPipelineLayout(VulkanGraphics* graphics, const ResourceLayout &layout);
        ~VulkanPipelineLayout();

        const ResourceLayout& GetResourceLayout() const { return _layout; }
        VkPipelineLayout GetVkHandle() const { return _vkHandle; }
        VulkanDescriptorSetAllocator* GetAllocator(uint32_t set) const
        {
            return _setAllocators[set];
        }

    private:
        VkDevice _logicalDevice;
        VkPipelineLayout _vkHandle = VK_NULL_HANDLE;
        ResourceLayout _layout;
        VulkanDescriptorSetAllocator* _setAllocators[MaxDescriptorSets] = {};
    };
}
