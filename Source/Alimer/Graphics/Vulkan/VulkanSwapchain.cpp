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

#include "VulkanSwapchain.h"
#include "VulkanGraphicsDevice.h"
#include "VulkanCommandBuffer.h"
#include "../Texture.h"
#include "VulkanRenderPass.h"
#include "VulkanConvert.h"
#include "../../Core/Log.h"

namespace Alimer
{
    VulkanSwapchain::VulkanSwapchain(VulkanGraphics* graphics, VkSurfaceKHR surface, uint32_t width, uint32_t height)
        : _graphics(graphics)
        , _surface(surface)
        , _size(width, height)
        , _physicalDevice(graphics->GetPhysicalDevice())
        , _logicalDevice(graphics->GetDevice())
        , _imageCount(0)
    {
        VkResult result = VK_SUCCESS;

        VkBool32 supported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, graphics->GetGraphicsQueueFamily(), surface, &supported);
        if (!supported)
        {
            ALIMER_LOGERROR("[Vulkan] - Swapchain surface is not supported by graphics queue.");
            return;
        }

        // Get list of supported surface formats
        uint32_t formatCount;
        vkThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, surface, &formatCount, NULL));
        assert(formatCount > 0);

        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
        vkThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, surface, &formatCount, surfaceFormats.data()));

        // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
        // there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
        if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
        {
            _swapchainFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
            _swapchainFormat.colorSpace = surfaceFormats[0].colorSpace;
        }
        else
        {
            // iterate over the list of available surface format and
            // check for the presence of VK_FORMAT_B8G8R8A8_UNORM
            bool found_B8G8R8A8_UNORM = false;
            for (auto&& surfaceFormat : surfaceFormats)
            {
                if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
                {
                    _swapchainFormat.format = surfaceFormat.format;
                    _swapchainFormat.colorSpace = surfaceFormat.colorSpace;
                    found_B8G8R8A8_UNORM = true;
                    break;
                }
            }

            // in case VK_FORMAT_B8G8R8A8_UNORM is not available
            // select the first available color format
            if (!found_B8G8R8A8_UNORM)
            {
                _swapchainFormat.format = surfaceFormats[0].format;
                _swapchainFormat.colorSpace = surfaceFormats[0].colorSpace;
            }
        }

        Resize(0, 0, true);
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        for (uint32_t i = 0; i < _imageCount; i++)
        {
            //vkDestroyImageView(_logicalDevice, buffers[i].view, nullptr);
        }

        if (_swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(_logicalDevice, _swapchain, nullptr);
        }

        if (_surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(_graphics->GetInstance(), _surface, nullptr);
        }

        _swapchain = VK_NULL_HANDLE;
        _surface = VK_NULL_HANDLE;
        _imageCount = 0;
    }

    void VulkanSwapchain::Resize(uint32_t width, uint32_t height, bool force)
    {
        if (_size.x == width &&
            _size.y == height &&
            !force) {
            return;
        }

        // Get physical device surface properties and formats
        VkSurfaceCapabilitiesKHR surfCaps;
        vkThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surfCaps));

        // Get available present modes
        uint32_t presentModeCount;
        vkThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, nullptr));
        assert(presentModeCount > 0);

        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, presentModes.data()));

        VkExtent2D swapchainExtent = {};
        // If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
        if (surfCaps.currentExtent.width == (uint32_t)-1)
        {
            // If the surface size is undefined, the size is set to
            // the size of the images requested.
            swapchainExtent.width = width;
            swapchainExtent.height = height;
        }
        else
        {
            // If the surface size is defined, the swap chain size must match
            swapchainExtent = surfCaps.currentExtent;
            width = surfCaps.currentExtent.width;
            height = surfCaps.currentExtent.height;
        }

        // Select a present mode for the swapchain

        // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
        // This mode waits for the vertical blank ("v-sync")
        VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

        // If v-sync is not requested, try to find a mailbox mode
        // It's the lowest latency non-tearing present mode available
        if (!_vsync)
        {
            for (size_t i = 0; i < presentModeCount; i++)
            {
                if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                    break;
                }
                if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
                {
                    swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                }
            }
        }

        // Determine the number of images
        uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
        if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
        {
            desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
        }

        // Find the transformation of the surface
        VkSurfaceTransformFlagsKHR preTransform;
        if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        {
            // We prefer a non-rotated transform
            preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        }
        else
        {
            preTransform = surfCaps.currentTransform;
        }

        // Find a supported composite alpha format (not all devices support alpha opaque)
        VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        // Simply select the first composite alpha format available
        std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        };
        for (auto& compositeAlphaFlag : compositeAlphaFlags) {
            if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) {
                compositeAlpha = compositeAlphaFlag;
                break;
            };
        }

        VkSwapchainKHR oldSwapchain = _swapchain;

        VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        createInfo.pNext = NULL;
        createInfo.surface = _surface;
        createInfo.minImageCount = desiredNumberOfSwapchainImages;
        createInfo.imageFormat = _swapchainFormat.format;
        createInfo.imageColorSpace = _swapchainFormat.colorSpace;
        createInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
        createInfo.imageArrayLayers = 1;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.presentMode = swapchainPresentMode;
        createInfo.oldSwapchain = oldSwapchain;
        // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
        createInfo.clipped = VK_TRUE;
        createInfo.compositeAlpha = compositeAlpha;

        // Enable transfer source on swap chain images if supported
        if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        // Enable transfer destination on swap chain images if supported
        if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        vkThrowIfFailed(vkCreateSwapchainKHR(_logicalDevice, &createInfo, nullptr, &_swapchain));

        _format = vk::Convert(_swapchainFormat.format);;
        _size.x = width;
        _size.y = height;

        // If an existing swap chain is re-created, destroy the old swap chain
        // This also cleans up all the presentable images
        if (oldSwapchain != VK_NULL_HANDLE)
        {
            for (uint32_t i = 0; i < _imageCount; i++)
            {
                //vkDestroyImageView(_logicalDevice, buffers[i].view, nullptr);
            }

            vkDestroySwapchainKHR(_logicalDevice, oldSwapchain, nullptr);
        }
        vkThrowIfFailed(vkGetSwapchainImagesKHR(_logicalDevice, _swapchain, &_imageCount, nullptr));

        // Get the swap chain images
        _vkImages.resize(_imageCount);
        _textures.resize(_imageCount);
        vkThrowIfFailed(vkGetSwapchainImagesKHR(_logicalDevice, _swapchain, &_imageCount, _vkImages.data()));

        TextureDescriptor textureDesc = {};
        textureDesc.type = TextureType::Type2D;
        textureDesc.usage = TextureUsage::RenderTarget;
        textureDesc.format = _format;
        textureDesc.width = _size.x;
        textureDesc.height = _size.y;

        for (uint32_t i = 0; i < _imageCount; i++)
        {
            _textures[i] = new Texture(_graphics);
            _textures[i]->SetVkImage(&textureDesc, _vkImages[i], createInfo.imageUsage);
        }

        if (createInfo.imageUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        {
            // Clear images with default color.
            VkClearColorValue clearColor = {};
            clearColor.float32[3] = 1.0f;

            VkImageSubresourceRange clearRange = {};
            clearRange.layerCount = 1;
            clearRange.levelCount = 1;

            VkCommandBuffer clearImageCmdBuffer = _graphics->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
            for (uint32_t i = 0; i < _imageCount; i++)
            {
                // Clear with default color.
                _graphics->ClearImageWithColor(
                    clearImageCmdBuffer,
                    _vkImages[i],
                    clearRange,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    VkAccessFlagBits(0),
                    &clearColor);
            }

            _graphics->FlushCommandBuffer(clearImageCmdBuffer, true);
        }
    }

    SharedPtr<TextureView> VulkanSwapchain::GetTextureView(uint32_t index) const
    {
        return _textures[index]->GetDefaultTextureView();
    }

    VkResult VulkanSwapchain::AcquireNextImage(uint32_t *pImageIndex, VkSemaphore* pImageAcquiredSemaphore)
    {
        // Acquire the next swapchain image.
        *pImageAcquiredSemaphore = _graphics->AcquireSemaphore();

        VkResult result = vkAcquireNextImageKHR(
            _logicalDevice,
            _swapchain,
            std::numeric_limits<uint64_t>::max(),
            *pImageAcquiredSemaphore,
            (VkFence)nullptr,
            pImageIndex);

        if (result != VK_SUCCESS)
        {
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            {
                Resize(_size.x, _size.y, true);
            }
        }

        return result;
    }

    VkResult VulkanSwapchain::QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
    {
        VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.pNext = nullptr;
        // Check if a wait semaphore has been specified to wait for before presenting the image
        if (waitSemaphore != VK_NULL_HANDLE)
        {
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &waitSemaphore;
        }
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &_swapchain;
        presentInfo.pImageIndices = &imageIndex;

        return vkQueuePresentKHR(queue, &presentInfo);
    }
}
