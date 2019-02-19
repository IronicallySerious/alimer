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

#include "../Texture.h"
#include "GraphicsDeviceVk.h"
#include "../../Core/Log.h"

namespace alimer
{
    static VkImageType GetVkImageType(TextureType type) {
        switch (type) {
        case TextureType::Type1D:
            return VK_IMAGE_TYPE_1D;

        case TextureType::Type2D:
        case TextureType::TypeCube:
            return VK_IMAGE_TYPE_2D;

        case TextureType::Type3D:
            return VK_IMAGE_TYPE_3D;
        default:
            ALIMER_UNREACHABLE();
            return VK_IMAGE_TYPE_MAX_ENUM;
        }
    }

    static VkImageUsageFlags GetVkTextureUsage(TextureUsage usage, PixelFormat format) {
        VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        /* TODO: Should we exposed VK_IMAGE_USAGE_TRANSFER_DST_BIT  for staging?
        if (any(usage & TextureUsage::TransferSrc)) {
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if (usage & VGPU_TEXTURE_USAGE_TRANSFER_DEST) {
            flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }*/

        if (any(usage & TextureUsage::ShaderRead)) {
            flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        if (any(usage & TextureUsage::ShaderWrite)) {
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        if (any(usage & TextureUsage::RenderTarget)) {
            if (IsDepthStencilFormat(format)) {
                flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }
            else {
                flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
        }

        return flags;
    }

    static VkImageViewType GetVkImageViewType(TextureType type, bool isArray) {
        switch (type) {
        case TextureType::Type1D:
            return isArray ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
        case TextureType::Type2D:
            return isArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
        case TextureType::Type3D:
            assert(isArray == false);
            return VK_IMAGE_VIEW_TYPE_3D;

        case TextureType::TypeCube:
            return isArray ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
        default:
            ALIMER_UNREACHABLE();
            return VK_IMAGE_VIEW_TYPE_2D;
        }
    }

    VkImageAspectFlags GetVkAspectFlags(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_UNDEFINED:
            return 0;

        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;

        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            return VK_IMAGE_ASPECT_DEPTH_BIT;

        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    /*TextureVk::TextureVk(GraphicsDevice* device, const TextureDescriptor* descriptor, VkImage nativeTexture, const void* pInitData)
        : Texture(device, descriptor)
        , _vkFormat(GetVkFormat(descriptor->format))
    {
        if (nativeTexture != VK_NULL_HANDLE)
        {
            _handle = nativeTexture;
            _externalHandle = true;
        }
    }*/

    bool Texture::Create(const void* pInitData)
    {
        VkImageUsageFlags vkUsage = GetVkTextureUsage(_usage, _format);

        if (!_externalHandle)
        {
            // Create vulkan texture.
            VkImageCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0u;
            createInfo.imageType = GetVkImageType(_type);
            createInfo.format = _vkFormat;
            createInfo.extent.width = _width;
            createInfo.extent.height = _height;
            createInfo.extent.depth = _depth;
            createInfo.mipLevels = _mipLevels;
            createInfo.arrayLayers = _arraySize;
            createInfo.samples = (VkSampleCountFlagBits)_samples;
            createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            createInfo.usage = vkUsage;
            createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
            createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            /* Cube maps */
            if (_type == TextureType::TypeCube
                && _width == _height) {
                createInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                createInfo.arrayLayers *= 6u;
            }

            VkResult result = VK_SUCCESS;
            //if ((memoryFlags & VGPU_MEMORY_NO_ALLOCATION) == 0) {
                // Determine appropriate memory usage flags.
                VmaAllocationCreateInfo allocCreateInfo = {};
                allocCreateInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;
                allocCreateInfo.flags = 0u;

                //if (memoryFlags == VGPU_MEMORY_GPU_ONLY) {
                    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                //}
                //else if (memoryFlags == VGPU_MEMORY_CPU_ONLY) {
                //    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
                //}
                //else if (memoryFlags == VGPU_MEMORY_CPU_TO_GPU) {
                //    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                //}
                //else if (memoryFlags == VGPU_MEMORY_GPU_TO_CPU) {
                //    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
                //}

                //if (memoryFlags & VGPU_MEMORY_DEDICATED_ALLOCATION) {
                //    allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
                //}

                    vkThrowIfFailed(
                        vmaCreateImage(_device->GetImpl()->GetVmaAllocator(), &createInfo, &allocCreateInfo, &_handle, &_allocation, nullptr)
                        );
            //}
            //else {
            //    vkThrowIfFailed(
            //        vkCreateImage(deviceVk->GetVkDevice(), &createInfo, nullptr, &texture->vk_handle)
            //        );
            //}
        }

        if (vkUsage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT))
        {
            VkImageViewCreateInfo viewCreateInfo = {};
            viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewCreateInfo.pNext = nullptr;
            viewCreateInfo.flags = 0u;
            viewCreateInfo.image = _handle;
            viewCreateInfo.viewType = GetVkImageViewType(_type, _arraySize > 1);
            viewCreateInfo.format = _vkFormat;
            viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
            viewCreateInfo.subresourceRange.aspectMask = GetVkAspectFlags(_vkFormat);
            viewCreateInfo.subresourceRange.baseMipLevel = 0;
            viewCreateInfo.subresourceRange.levelCount = _mipLevels;
            viewCreateInfo.subresourceRange.baseArrayLayer = 0;
            viewCreateInfo.subresourceRange.layerCount = _arraySize;

            if (vkCreateImageView(_device->GetImpl()->GetVkDevice(), &viewCreateInfo, nullptr, &_defaultImageView) != VK_SUCCESS)
            {
                ALIMER_LOGERROR("[Vulkan] - Failed to create ImageView.");
            }
        }

        /* TODO */
        return true;
    }

    void Texture::Destroy()
    {
        if (_defaultImageView != VK_NULL_HANDLE)
        {
            //deviceVk->DestroyImageView(_vkImageView);
            vkDestroyImageView(_device->GetImpl()->GetVkDevice(), _defaultImageView, nullptr);
            _defaultImageView = VK_NULL_HANDLE;
        }

        if (!_externalHandle
            && _handle != VK_NULL_HANDLE)
        {
            if (_allocation != VK_NULL_HANDLE) {
                vmaDestroyImage(_device->GetImpl()->GetVmaAllocator(), _handle, _allocation);
            }
            else {
                _device->GetImpl()->DestroyImage(_handle);
            }
            
            _handle = VK_NULL_HANDLE;
        }
    }

    VkImageView Texture::GetView(uint32_t level, uint32_t slice) const
    {
        if (level == 0 && slice == 0) {
            return _defaultImageView;
        }

        /* TODO : handle level and slice. */
        return VK_NULL_HANDLE;
    }
}
