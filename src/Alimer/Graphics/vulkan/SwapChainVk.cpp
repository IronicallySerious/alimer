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

#include "../GraphicsDevice.h"
#include "../PhysicalDevice.h"
#include "../SwapChain.h"
#include "VulkanConvert.h"
#include "../../Core/Log.h"

namespace alimer
{
    void SwapChain::PlatformDestroy()
    {

    }

    bool SwapChain::Define(VkSurfaceKHR surface, const SwapChainDescriptor* descriptor)
    {
        _surface = surface;
        _depthStencilFormat = descriptor->preferredDepthStencilFormat;
        return Resize(descriptor->width, descriptor->height);
    }

    bool SwapChain::PlatformResize(uint32_t width, uint32_t height)
    {
        VkPhysicalDevice  physicalDevice = _graphicsDevice->GetPhysicalDevice()->GetHandle();

        VkSurfaceCapabilitiesKHR surfaceCaps;
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, _surface, &surfaceCaps) != VK_SUCCESS)
        {
            return false;
        }

        // Check windows size.
        if (surfaceCaps.maxImageExtent.width == 0 &&
            surfaceCaps.maxImageExtent.height == 0)
        {
            return false;
        }

        VkBool32 supported = VK_FALSE;

        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, _graphicsDevice->GetVkGraphicsQueueFamily(), _surface, &supported);
        if (!supported)
        {
            return false;
        }

        // No surface, should sleep and retry maybe?.
        if (surfaceCaps.maxImageExtent.width == 0 &&
            surfaceCaps.maxImageExtent.height == 0)
        {
            return nullptr;
        }

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, formats.data());

        VkSurfaceFormatKHR format = { VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
        if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        {
            format = formats[0];
            format.format = VK_FORMAT_B8G8R8A8_UNORM;
        }
        else
        {
            if (formatCount == 0)
            {
                ALIMER_LOGERROR("Surface has no formats.");
                return nullptr;
            }

            bool found = false;
            for (uint32_t i = 0; i < formatCount; i++)
            {
                if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB ||
                    formats[i].format == VK_FORMAT_B8G8R8A8_SRGB ||
                    formats[i].format == VK_FORMAT_A8B8G8R8_SRGB_PACK32)
                {
                    format = formats[i];
                    found = true;
                }
            }

            if (!found)
            {
                format = formats[0];
            }
        }

        VkExtent2D swapchainSize;
        if (surfaceCaps.currentExtent.width == ~0u)
        {
            swapchainSize.width = width;
            swapchainSize.height = height;
        }
        else
        {
            swapchainSize.width = Max(Min(width, surfaceCaps.maxImageExtent.width), surfaceCaps.minImageExtent.width);
            swapchainSize.height = Max(Min(height, surfaceCaps.maxImageExtent.height), surfaceCaps.minImageExtent.height);
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, presentModes.data());

        VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        bool useVsync = true;
        if (!useVsync)
        {
            for (uint32_t i = 0; i < presentModeCount; i++)
            {
                if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR
                    || presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    swapchainPresentMode = presentModes[i];
                    break;
                }
            }
        }

        uint32_t desiredNumberOfSwapchainImages = 3;
        /* TODO: Allow customization.
        {
            const char *num_images = getenv("VGPU_VULKAN_SWAPCHAIN_IMAGES");
            if (num_images)
                desiredNumberOfSwapchainImages = uint32_t(strtoul(num_images, nullptr, 0));
        }
        */

        if (desiredNumberOfSwapchainImages < surfaceCaps.minImageCount)
            desiredNumberOfSwapchainImages = surfaceCaps.minImageCount;

        if ((surfaceCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCaps.maxImageCount))
        {
            desiredNumberOfSwapchainImages = surfaceCaps.maxImageCount;
        }

        ALIMER_LOGINFO("Targeting {} swapchain images.", desiredNumberOfSwapchainImages);

        VkSurfaceTransformFlagBitsKHR preTransform;
        if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        else
            preTransform = surfaceCaps.currentTransform;

        VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        if (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
            compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        if (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
            compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        if (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
            compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
        if (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
            compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

        VkSwapchainKHR oldSwapchain = _handle;

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.surface = _surface;
        createInfo.minImageCount = desiredNumberOfSwapchainImages;
        createInfo.imageFormat = format.format;
        createInfo.imageColorSpace = format.colorSpace;
        createInfo.imageExtent.width = swapchainSize.width;
        createInfo.imageExtent.height = swapchainSize.height;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
        createInfo.preTransform = preTransform;
        createInfo.compositeAlpha = compositeAlpha;
        createInfo.presentMode = swapchainPresentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapchain;

        TextureUsage textureUsage = TextureUsage::RenderTarget;

        // Enable transfer source on swap chain images if supported.
        if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            textureUsage |= TextureUsage::TransferSrc;
        }

        // Enable transfer destination on swap chain images if supported
        if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            textureUsage |= TextureUsage::TransferDest;
        }

        VkResult result = vkCreateSwapchainKHR(_graphicsDevice->GetVkDevice(), &createInfo, nullptr, &_handle);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan: Failed to create swapchain, error: {}", vkGetVulkanResultString(result));
            return false;
        }

        if (oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(_graphicsDevice->GetVkDevice(), oldSwapchain, nullptr);
        }

        uint32_t imageCount;
        vkGetSwapchainImagesKHR(_graphicsDevice->GetVkDevice(), _handle, &imageCount, nullptr);
        std::vector<VkImage> swapchainImages(imageCount);
        vkGetSwapchainImagesKHR(_graphicsDevice->GetVkDevice(), _handle, &imageCount, swapchainImages.data());

        ALIMER_LOGINFO("Vulkan: Created swapchain {} x {} (imageCount: {}, format: {}).",
            swapchainSize.width,
            swapchainSize.height,
            imageCount,
            static_cast<unsigned>(format.format));

        _width = swapchainSize.width;
        _height = swapchainSize.height;
        _colorFormat = vk::Convert(format.format);
        _backbufferTextures.Resize(imageCount);
        for (uint32_t i = 0; i < imageCount; ++i)
        {
            _backbufferTextures[i] = new Texture(_graphicsDevice);
            _backbufferTextures[i]->DefineFromHandle(
                swapchainImages[i], 
                TextureType::Type2D,
                swapchainSize.width, swapchainSize.height, 
                1, 1, 1,
                _colorFormat,
                textureUsage,
                SampleCount::Count1);
        }

        

        return true;
    }

    void SwapChain::PlatformPresent()
    {

    }
}
