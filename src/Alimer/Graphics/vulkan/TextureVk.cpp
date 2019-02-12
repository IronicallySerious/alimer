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

#include "TextureVk.h"
#include "GPUDeviceVk.h"
#include "../../Core/Log.h"

namespace alimer
{
    static VkImageUsageFlags VkImageUsage(TextureUsage usage, PixelFormat format)
    {
        VkImageUsageFlags flags = 0;

        if (any(usage & TextureUsage::TransferSrc)) {
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if (any(usage & TextureUsage::TransferDest)) {
            flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        if (any(usage & TextureUsage::Sampled)) {
            flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        if (any(usage & TextureUsage::Storage)) {
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        if (any(usage & TextureUsage::RenderTarget))
        {
            if (IsDepthStencilFormat(format)) {
                flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }
            else {
                flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
        }

        return flags;
    }

    TextureVk::TextureVk(GPUDeviceVk* device, const TextureDescriptor* descriptor, void* nativeTexture, const void* pInitData)
        : Texture(device, descriptor)
        , _device(device)
    {
        if (nativeTexture != nullptr)
        {
            _handle = static_cast<VkImage>(nativeTexture);
            _externalHandle = true;
        }
        else
        {

        }
    }

    TextureVk::~TextureVk()
    {
        Destroy();
    }

    void TextureVk::Destroy()
    {
        if (!_externalHandle
            && _handle != VK_NULL_HANDLE)
        {
            _device->DestroyImage(_handle);
            _handle = VK_NULL_HANDLE;
        }
    }

    /*VulkanTexture::VulkanTexture(VulkanGraphicsDevice* device, const TextureDescriptor* descriptor, const ImageLevel* initialData, VkImage existingImage, VkImageUsageFlags usageFlags)
        : Texture(device, descriptor)
        , _logicalDevice(device->GetDevice())
        , _allocator(device->GetVmaAllocator())
        , _vkImage(existingImage)
        , _allocated(existingImage == VK_NULL_HANDLE)
    {
        if (usageFlags & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT))
        {
            //TextureViewDescriptor viewDescriptor = {};
            //viewDescriptor.format = descriptor->format;
            //_defaultTextureView = CreateTextureView(&viewDescriptor);
        }
    }

    VulkanTextureView::VulkanTextureView(VulkanGraphicsDevice* device, VulkanTexture* texture, const TextureViewDescriptor* descriptor)
        : TextureView(texture, descriptor)
        , _logicalDevice(device->GetDevice())
        , _id(device->AllocateCookie())
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.image = texture->GetVkImage();
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // get_image_view_type(tmpinfo, nullptr);
        createInfo.format = vk::Convert(descriptor->format);
        createInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        createInfo.subresourceRange = {
            vk::FormatToAspectMask(createInfo.format),
            descriptor->baseMipLevel, descriptor->levelCount,
            descriptor->baseArrayLayer, descriptor->layerCount
        };

        if (vkCreateImageView(
            _logicalDevice,
            &createInfo,
            nullptr, &_vkImageView) != VK_SUCCESS)
        {
            ALIMER_LOGERROR("[Vulkan] - Failed to create ImageView.");
        }
    }

    VulkanTextureView::~VulkanTextureView()
    {
        if (_vkImageView != VK_NULL_HANDLE)
        {
            //_texture->GetGraphics()->DestroyImageView(_vkImageView);
            vkDestroyImageView(_logicalDevice, _vkImageView, nullptr);
            _vkImageView = VK_NULL_HANDLE;
        }
    }*/
}
