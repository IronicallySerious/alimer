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

#include "VulkanPrerequisites.h"
#include "../Types.h"
#include "../PixelFormat.h"

namespace Alimer
{
    namespace vk
    {
        // Put an image memory barrier for setting an image layout on the sub resource into the given command buffer
        void SetImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkImageSubresourceRange subresourceRange,
            VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        // Uses a fixed sub resource layout with first mip level and layer
        void SetImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkImageAspectFlags aspectMask,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        static inline VkFormat Convert(PixelFormat format)
        {
            // VK_FORMAT_X8_D24_UNORM_PACK32
            switch (format)
            {
                case PixelFormat::R8UNorm:              return VK_FORMAT_R8_UNORM;
                case PixelFormat::RG8UNorm:              return VK_FORMAT_R8G8_UNORM;
                case PixelFormat::RGBA8UNorm:			return VK_FORMAT_R8G8B8A8_UNORM;
                case PixelFormat::BGRA8UNorm:			return VK_FORMAT_B8G8R8A8_UNORM;
                //case PixelFormat::BGRA8UNorm_SRGB:		return VK_FORMAT_B8G8R8A8_SRGB;
                //case PixelFormat::RGBA8UNorm_SRGB:		return VK_FORMAT_R8G8B8A8_SRGB;
                //case PixelFormat::Depth16UNorm:			return VK_FORMAT_D16_UNORM;
                //case PixelFormat::Depth16UNormStencil8:	return VK_FORMAT_D16_UNORM_S8_UINT;
                //case PixelFormat::Depth32Float:			return VK_FORMAT_D32_SFLOAT;
                //case PixelFormat::Stencil8:				return VK_FORMAT_S8_UINT;
                //case PixelFormat::Depth24UNormStencil8:	return VK_FORMAT_D24_UNORM_S8_UINT;
                //case PixelFormat::Depth32FloatStencil8: return VK_FORMAT_D32_SFLOAT_S8_UINT;
                //case PixelFormat::BC1:					return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
                //case PixelFormat::BC1_SRGB:				return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
                //case PixelFormat::BC2:					return VK_FORMAT_BC2_UNORM_BLOCK;
                //case PixelFormat::BC2_SRGB:				return VK_FORMAT_BC2_SRGB_BLOCK;
                //case PixelFormat::BC3:					return VK_FORMAT_BC3_UNORM_BLOCK;
                //case PixelFormat::BC3_SRGB:				return VK_FORMAT_BC3_SRGB_BLOCK;
                default:
                    return VK_FORMAT_UNDEFINED;
            }
        }

        static inline VkAttachmentLoadOp Convert(LoadAction action)
        {
            switch (action)
            {
                case LoadAction::DontCare:
                    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;

                case LoadAction::Load:
                    return VK_ATTACHMENT_LOAD_OP_LOAD;

                case LoadAction::Clear:
                    return VK_ATTACHMENT_LOAD_OP_CLEAR;

                default:
                    return VK_ATTACHMENT_LOAD_OP_LOAD;
            }
        }

        static inline VkAttachmentStoreOp Convert(StoreAction action)
        {
            switch (action)
            {
                case StoreAction::Store:
                    return VK_ATTACHMENT_STORE_OP_STORE;

                case StoreAction::DontCare:
                    return VK_ATTACHMENT_STORE_OP_DONT_CARE;

                default:
                    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }
        }
    }
}
