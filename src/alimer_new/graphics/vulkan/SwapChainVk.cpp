//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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

#include "SwapChainVk.h"
#include "GraphicsDeviceVk.h"
#include <algorithm>
using namespace std;

namespace alimer
{
    SwapChainVk::SwapChainVk(GraphicsDeviceVk* device, VkSurfaceKHR surface, const SwapChainDescriptor* descriptor)
        : SwapChain(device, descriptor)
        , _surface(surface)
        , _instance(device->GetVkInstance())
        , _physicalDevice(device->GetVkPhysicalDevice())
        , _device(device->GetVkDevice())
    {
        ResizeImpl(descriptor->width, descriptor->height);
    }

    SwapChainVk::~SwapChainVk()
    {
        Destroy();
    }

    void SwapChainVk::Destroy()
    {
        if (_swapchain != VK_NULL_HANDLE)
        {
            for (uint32_t i = 0; i < _imageCount; i++)
            {
                //vkDestroyImageView(device, buffers[i].view, nullptr);
                vkDestroySemaphore(_device, _imageSemaphores[i], nullptr);
            }

            vkDestroySwapchainKHR(_device, _swapchain, nullptr);
            _swapchain = VK_NULL_HANDLE;
        }


        if (_surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(_instance, _surface, nullptr);
            _surface = VK_NULL_HANDLE;
        }
    }

    bool SwapChainVk::ResizeImpl(uint32_t width, uint32_t height)
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surfaceCapabilities));

        uint32_t count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &count, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &count, formats.data());

        if (count == 1
            && formats[0].format == VK_FORMAT_UNDEFINED)
        {
            _format = formats[0];
            _format.format = VK_FORMAT_B8G8R8A8_UNORM;
            _colorFormat = PixelFormat::BGRA8UNorm;
        }
        else
        {
            if (count == 0)
            {
                LOGE("Vulkan: Surface has no formats.");
                return false;
            }

            bool found = false;
            for (uint32_t i = 0; i < count; i++)
            {
                if (_srgb)
                {
                    if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB ||
                        formats[i].format == VK_FORMAT_B8G8R8A8_SRGB ||
                        formats[i].format == VK_FORMAT_A8B8G8R8_SRGB_PACK32)
                    {
                        _format = formats[i];
                        found = true;
                    }
                }
                else
                {
                    if (formats[i].format == VK_FORMAT_R8G8B8A8_UNORM ||
                        formats[i].format == VK_FORMAT_B8G8R8A8_UNORM ||
                        formats[i].format == VK_FORMAT_A8B8G8R8_UNORM_PACK32)
                    {
                        _format = formats[i];
                        found = true;
                    }
                }
            }

            if (!found)
            {
                _format = formats[0];
                _colorFormat = PixelFormat::BGRA8UNorm;
            }
        }

        VkExtent2D swapchainSize;
        if (surfaceCapabilities.currentExtent.width == ~0u)
        {
            swapchainSize.width = width;
            swapchainSize.height = height;
        }
        else
        {
            swapchainSize.width = max(min(width, surfaceCapabilities.maxImageExtent.width), surfaceCapabilities.minImageExtent.width);
            swapchainSize.height = max(min(height, surfaceCapabilities.maxImageExtent.height), surfaceCapabilities.minImageExtent.height);
        }

        vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &count, nullptr);
        vector<VkPresentModeKHR> present_modes(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &count, present_modes.data());

        VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;
        if (!_vSyncEnabled)
        {
            for (uint32_t i = 0; i < count; i++)
            {
                if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR || present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    swapchain_present_mode = present_modes[i];
                    break;
                }
            }
        }

        uint32_t desired_swapchain_images = 3;

        if (desired_swapchain_images < surfaceCapabilities.minImageCount)
            desired_swapchain_images = surfaceCapabilities.minImageCount;

        if ((surfaceCapabilities.maxImageCount > 0) && (desired_swapchain_images > surfaceCapabilities.maxImageCount))
            desired_swapchain_images = surfaceCapabilities.maxImageCount;

        VkSurfaceTransformFlagBitsKHR pre_transform;
        if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        else
            pre_transform = surfaceCapabilities.currentTransform;

        VkCompositeAlphaFlagBitsKHR composite_mode = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
            composite_mode = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
            composite_mode = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
            composite_mode = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
            composite_mode = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

        VkSwapchainKHR old_swapchain = _swapchain;

        VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        createInfo.surface = _surface;
        createInfo.minImageCount = desired_swapchain_images;
        createInfo.imageFormat = _format.format;
        createInfo.imageColorSpace = _format.colorSpace;
        createInfo.imageExtent.width = swapchainSize.width;
        createInfo.imageExtent.height = swapchainSize.height;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = pre_transform;
        createInfo.compositeAlpha = composite_mode;
        createInfo.presentMode = swapchain_present_mode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = old_swapchain;

        // Enable transfer source on swap chain images if supported
        if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        // Enable transfer destination on swap chain images if supported
        if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain) != VK_SUCCESS)
        {
            return false;
        }

        if (old_swapchain != VK_NULL_HANDLE)
        {
            for (uint32_t i = 0; i < _imageCount; i++)
            {
                //vkDestroyImageView(device, buffers[i].view, nullptr);
                vkDestroySemaphore(_device, _imageSemaphores[i], nullptr);
            }

            vkDestroySwapchainKHR(_device, old_swapchain, nullptr);
        }

        vkThrowIfFailed(vkGetSwapchainImagesKHR(_device, _swapchain, &_imageCount, nullptr));
        _images.resize(_imageCount);
        _imageSemaphores.resize(_imageCount);
        vkThrowIfFailed(vkGetSwapchainImagesKHR(_device, _swapchain, &_imageCount, _images.data()));

        // Create command buffer for transition or clear
        auto deviceVk = static_cast<GraphicsDeviceVk*>(_graphicsDevice);
        auto setupSwapchainCmdBuffer = deviceVk->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        for (uint32_t i = 0; i < _imageCount; i++)
        {
            VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            vkThrowIfFailed(
                vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_imageSemaphores[i])
            );

            // Clear with default value if supported.
            if (createInfo.imageUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            {
                // Clear images with default color.
                VkClearColorValue clearColor = {};
                clearColor.float32[3] = 1.0f;
                //clearColor.float32[0] = 1.0f;

                VkImageSubresourceRange clearRange = {};
                clearRange.layerCount = 1;
                clearRange.levelCount = 1;

                // Clear with default color.
                vkClearImageWithColor(
                    setupSwapchainCmdBuffer,
                    _images[i],
                    clearRange,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    &clearColor);
            }
            else
            {
                // Transition image to present layout.
                vkTransitionImageLayout(
                    setupSwapchainCmdBuffer,
                    _images[i],
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                );
            }
        }

        deviceVk->FlushCommandBuffer(setupSwapchainCmdBuffer, true);
    }
}
