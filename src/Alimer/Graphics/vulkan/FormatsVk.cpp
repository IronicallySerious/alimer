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

#include "BackendVk.h"

namespace alimer
{
    const VkFormatDesc s_vkFormatDesc[] =
    {
        { PixelFormat::Unknown,                     VK_FORMAT_UNDEFINED },

        // 8-bit pixel formats
        { PixelFormat::A8UNorm,                     VK_FORMAT_R8_UNORM },
        { PixelFormat::R8UNorm,                     VK_FORMAT_R8_UNORM },
        { PixelFormat::R8SNorm,                     VK_FORMAT_R8_SNORM },
        { PixelFormat::R8UInt,                      VK_FORMAT_R8_UINT },
        { PixelFormat::R8SInt,                      VK_FORMAT_R8_SINT },

        // 16-bit pixel formats
        { PixelFormat::R16UNorm,                    VK_FORMAT_R16_UNORM },
        { PixelFormat::R16SNorm,                    VK_FORMAT_R16_SNORM },
        { PixelFormat::R16SInt,                     VK_FORMAT_R16_SINT },
        { PixelFormat::R16UInt,                     VK_FORMAT_R16_UINT },
        { PixelFormat::R16Float,                    VK_FORMAT_R16_SFLOAT },
        { PixelFormat::RG8UNorm,                    VK_FORMAT_R8G8_UNORM },
        { PixelFormat::RG8SNorm,                    VK_FORMAT_R8G8_SNORM },
        { PixelFormat::RG8UInt,                     VK_FORMAT_R8G8_UINT },
        { PixelFormat::RG8SInt,                     VK_FORMAT_R8G8_SINT },

        // Packed 16-bit pixel formats
        { PixelFormat::R5G6B5UNorm,                 VK_FORMAT_B5G6R5_UNORM_PACK16}, // DXGI_FORMAT_B5G6R5_UNORM 
        { PixelFormat::RGBA4UNorm,                  VK_FORMAT_B4G4R4A4_UNORM_PACK16}, // DXGI_FORMAT_B4G4R4A4_UNORM
       
        // 32-bit pixel formats
        { PixelFormat::R32UInt,                     VK_FORMAT_R32_UINT}, 
        { PixelFormat::R32SInt,                     VK_FORMAT_R32_SINT}, 
        { PixelFormat::R32Float,                    VK_FORMAT_R32_SFLOAT},
        { PixelFormat::RG16UNorm,                   VK_FORMAT_R16G16_UNORM },
        { PixelFormat::RG16SNorm,                   VK_FORMAT_R16G16_SNORM },
        { PixelFormat::RG16UInt,                    VK_FORMAT_R16G16_UINT },
        { PixelFormat::RG16SInt,                    VK_FORMAT_R16G16_SINT },
        { PixelFormat::RG16Float,                   VK_FORMAT_R16G16_SFLOAT },
        { PixelFormat::RGBA8UNorm,                  VK_FORMAT_R8G8B8A8_UNORM },
        //{ PixelFormat::RGBA8UNormSrgb,              VK_FORMAT_R8G8B8A8_SRGB },
        { PixelFormat::RGBA8SNorm,                  VK_FORMAT_R8G8B8A8_SNORM },
        { PixelFormat::RGBA8UInt,                   VK_FORMAT_R8G8B8A8_UINT },
        { PixelFormat::RGBA8SInt,                   VK_FORMAT_R8G8B8A8_SINT },
        { PixelFormat::BGRA8UNorm,                  VK_FORMAT_B8G8R8A8_UNORM },
        //{ PixelFormat::BGRA8UNormSrgb,              VK_FORMAT_B8G8R8A8_SRGB },

        // Packed 32-Bit Pixel Formats
        { PixelFormat::RGB10A2UNorm,                VK_FORMAT_A2R10G10B10_UNORM_PACK32},
        { PixelFormat::RGB10A2UInt,                 VK_FORMAT_A2R10G10B10_UINT_PACK32},
        { PixelFormat::RG11B10Float,                VK_FORMAT_B10G11R11_UFLOAT_PACK32},
        { PixelFormat::RGB9E5Float,                 VK_FORMAT_E5B9G9R9_UFLOAT_PACK32},

        // Depth-stencil
        //{ PixelFormat::D32Float,                      VK_FORMAT_D32_SFLOAT },
        //{ PixelFormat::D16UNorm,                      VK_FORMAT_D16_UNORM },
        //{ PixelFormat::D24UNormS8,                    VK_FORMAT_D24_UNORM_S8_UINT },
        //{ PixelFormat::D32FloatS8,                      VK_FORMAT_D32_SFLOAT_S8_UINT },

        // Compressed formats
        { PixelFormat::BC1UNorm,                      VK_FORMAT_BC1_RGB_UNORM_BLOCK },
        //{ PixelFormat::BC1UNormSrgb,                  VK_FORMAT_BC1_RGB_SRGB_BLOCK },
        { PixelFormat::BC2UNorm,                      VK_FORMAT_BC2_UNORM_BLOCK },
        //{ PixelFormat::BC2UNormSrgb,                  VK_FORMAT_BC2_SRGB_BLOCK },
        { PixelFormat::BC3UNorm,                      VK_FORMAT_BC3_UNORM_BLOCK },
        //{ PixelFormat::BC3UNormSrgb,                  VK_FORMAT_BC3_SRGB_BLOCK },
        { PixelFormat::BC4UNorm,                      VK_FORMAT_BC4_UNORM_BLOCK },
        { PixelFormat::BC4SNorm,                      VK_FORMAT_BC4_SNORM_BLOCK },
        { PixelFormat::BC5UNorm,                      VK_FORMAT_BC5_UNORM_BLOCK },
        { PixelFormat::BC5SNorm,                      VK_FORMAT_BC5_SNORM_BLOCK },
        { PixelFormat::BC6HS16,                       VK_FORMAT_BC6H_SFLOAT_BLOCK },
        { PixelFormat::BC6HU16,                       VK_FORMAT_BC6H_UFLOAT_BLOCK },
        { PixelFormat::BC7UNorm,                      VK_FORMAT_BC7_UNORM_BLOCK },
        //{ PixelFormat::BC7UNormSrgb,                  VK_FORMAT_BC7_SRGB_BLOCK },
    };

    // Create an image memory barrier for changing the layout of
    // an image and put it into an active command buffer
    // See chapter 11.4 "Image Layout" for details

    void vkTransitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask)
    {
        // Create an image barrier object
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        // Source layouts (old)
        // Source access mask controls actions that have to be finished on the old layout
        // before it will be transitioned to the new layout
        switch (oldImageLayout)
        {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            // Image layout is undefined (or does not matter)
            // Only valid as initial layout
            // No flags required, listed only for completeness
            imageMemoryBarrier.srcAccessMask = 0;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            // Image is preinitialized
            // Only valid as initial layout for linear images, preserves memory contents
            // Make sure host writes have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image is a color attachment
            // Make sure any writes to the color buffer have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image is a depth/stencil attachment
            // Make sure any writes to the depth/stencil buffer have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image is a transfer source 
            // Make sure any reads from the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image is a transfer destination
            // Make sure any writes to the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image is read by a shader
            // Make sure any shader reads from the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
        switch (newImageLayout)
        {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image will be used as a transfer destination
            // Make sure any writes to the image have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image will be used as a transfer source
            // Make sure any reads from the image have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image will be used as a color attachment
            // Make sure any writes to the color buffer have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image layout will be used as a depth/stencil attachment
            // Make sure any writes to depth/stencil buffer have been finished
            imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image will be read in a shader (sampler, input attachment)
            // Make sure any writes to the image have been finished
            if (imageMemoryBarrier.srcAccessMask == 0)
            {
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
        }

        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(
            commandBuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
    }

    // Fixed sub resource on first mip level and layer
    void vkTransitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask)
    {
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = aspectMask;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;
        vkTransitionImageLayout(commandBuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
    }

    void vkClearImageWithColor(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageSubresourceRange range,
        VkImageAspectFlags aspect,
        VkImageLayout sourceLayout,
        VkImageLayout destLayout,
        VkClearColorValue *clearValue)
    {
        // Transition to destination layout.
        vkTransitionImageLayout(commandBuffer, image, aspect, sourceLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Clear the image
        range.aspectMask = aspect;
        vkCmdClearColorImage(
            commandBuffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            clearValue,
            1,
            &range);

        // Transition back to source layout.
        vkTransitionImageLayout(commandBuffer, image, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, destLayout);
    }
}
