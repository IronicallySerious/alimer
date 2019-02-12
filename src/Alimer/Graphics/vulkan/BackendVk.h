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

#include "volk.h"
#include "vk_mem_alloc.h"
#include "../Backend.h"

namespace alimer
{
    class GPUDeviceVk;

    struct DeviceFeaturesVk
    {
        bool supportsPhysicalDeviceProperties2 = false;
        bool supportsExternal = false;
        bool supportsDedicated = false;
        bool supportsImageFormatList = false;
        bool supportsDebugMarker = false;
        bool supportsDebugUtils = false;
        bool supportsMirrorClampToEdge = false;
        bool supportsGoogleDisplayTiming = false;
        bool supportsVulkan11Instance = false;
        bool supportsVulkan11Device = false;
        VkPhysicalDeviceSubgroupProperties subgroupProperties = {};
        VkPhysicalDevice8BitStorageFeaturesKHR storage8BitFeatures = {};
        VkPhysicalDevice16BitStorageFeaturesKHR storage16BitFeatures = {};
        VkPhysicalDeviceFloat16Int8FeaturesKHR float16Int8Features = {};
        VkPhysicalDeviceFeatures enabledFeatures = {};
    };

    

    struct VkFormatDesc
    {
        PixelFormat format;
        VkFormat    vkFormat;
    };

    extern ALIMER_API const VkFormatDesc s_vkFormatDesc[];

    inline VkFormat GetVkFormat(PixelFormat format)
    {
        assert(s_vkFormatDesc[(uint32_t)format].format == format);
        assert(s_vkFormatDesc[(uint32_t)format].vkFormat != VK_FORMAT_UNDEFINED);
        return s_vkFormatDesc[(uint32_t)format].vkFormat;
    }

    inline VkCompareOp GetVkCompareOp(CompareFunction function)
    {
        switch (function)
        {
        case CompareFunction::Never:
            return VK_COMPARE_OP_NEVER;
        case CompareFunction::Less:
            return VK_COMPARE_OP_LESS;
        case CompareFunction::Equal:
            return VK_COMPARE_OP_EQUAL;
        case CompareFunction::LessEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareFunction::Greater:
            return VK_COMPARE_OP_GREATER;
        case CompareFunction::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case CompareFunction::GreaterEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareFunction::Always:
            return VK_COMPARE_OP_ALWAYS;
        default:
            ALIMER_UNREACHABLE();
            return VK_COMPARE_OP_ALWAYS;
        }
    }

    // Put an image memory barrier for setting an image layout on the sub resource into the given command buffer
    void vkTransitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    // Uses a fixed sub resource layout with first mip level and layer
    void vkTransitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    void vkClearImageWithColor(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageSubresourceRange range,
        VkImageAspectFlags aspect,
        VkImageLayout sourceLayout,
        VkImageLayout destLayout,
        VkClearColorValue *clearValue);
}
