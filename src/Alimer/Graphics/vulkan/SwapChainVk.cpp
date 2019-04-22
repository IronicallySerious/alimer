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

#if TODO_VK
#include "SwapChainVk.h"
#include "FramebufferVk.h"
#include "GraphicsDeviceVk.h"
#include "../../Core/Log.h"

using namespace std;

namespace alimer
{
    SwapChainVk::SwapChainVk(GraphicsImpl* device, VkSurfaceKHR surface, const SwapChainDescriptor* descriptor)
        : _device(device)
        , _surface(surface)
        , _vSync(descriptor->vsync)
        , _samples(descriptor->samples)
    {
        if (descriptor->depthStencil) {
            _depthStencilFormat = device->GetDefaultDepthStencilFormat();
            if (_depthStencilFormat == PixelFormat::Undefined) {
                _depthStencilFormat = device->GetDefaultDepthFormat();
            }
        }

        //Resize(descriptor->width, descriptor->height);
    }

    void SwapChain::Destroy()
    {
        if (_handle != VK_NULL_HANDLE)
        {
            for (uint32_t i = 0u; i < _imageCount; i++)
            {
                vkDestroySemaphore(_device->GetImpl()->GetVkDevice(), _imageSemaphores[i], nullptr);
                //    vkDestroyImageView(_device->GetVkDevice(), buffers[i].view, nullptr);
            }

            vkDestroySwapchainKHR(_device->GetImpl()->GetVkDevice(), _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }

        if (_surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(_device->GetImpl()->GetVkInstance(), _surface, nullptr);
            _surface = VK_NULL_HANDLE;
        }
    }

    bool SwapChain::ResizeImpl(uint32_t width, uint32_t height)
    {
        VkPhysicalDevice physicalDevice = _device->GetImpl()->GetVkPhysicalDevice();

        // Create platform surface first.
        if (_surface == VK_NULL_HANDLE)
        {
            _surface = _device->GetImpl()->CreateSurface(_nativeHandle);
        }

        VkSurfaceCapabilitiesKHR surfaceCaps;
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, _surface, &surfaceCaps) != VK_SUCCESS)
        {
            return false;
        }

        // No surface, should sleep and retry maybe?.
        if (surfaceCaps.maxImageExtent.width == 0 &&
            surfaceCaps.maxImageExtent.height == 0)
        {
            return false;
        }

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        _presentQueueIndex = VK_QUEUE_FAMILY_IGNORED;
        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, _surface, &presentSupport);

            if (queueFamilies[i].queueCount > 0 && presentSupport) {
                _presentQueueIndex = i;
                break;
            }
        }

        VkDevice vkDevice = _device->GetImpl()->GetVkDevice();
        vkGetDeviceQueue(vkDevice, _presentQueueIndex, 0, &_presentQueue);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, nullptr);
        vector<VkSurfaceFormatKHR> formats(formatCount);
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
                return false;
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
            swapchainSize.width = max(min(width, surfaceCaps.maxImageExtent.width), surfaceCaps.minImageExtent.width);
            swapchainSize.height = max(min(height, surfaceCaps.maxImageExtent.height), surfaceCaps.minImageExtent.height);
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, nullptr);
        vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, presentModes.data());

        VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        if (!_vsync)
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

        uint32_t desiredNumberOfSwapchainImages = surfaceCaps.minImageCount + 1;
        if ((surfaceCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCaps.maxImageCount))
        {
            desiredNumberOfSwapchainImages = surfaceCaps.maxImageCount;
        }

        ALIMER_LOGDEBUG("Vulkan: Targeting {} swapchain images.", desiredNumberOfSwapchainImages);

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

        uint32_t queueFamilyIndices[] = { _device->GetImpl()->GetGraphicsQueueFamily(), _presentQueueIndex };

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
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT/* | VK_IMAGE_USAGE_SAMPLED_BIT*/;
        if (_device->GetImpl()->GetGraphicsQueueFamily() != _presentQueueIndex) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = preTransform;
        createInfo.compositeAlpha = compositeAlpha;
        createInfo.presentMode = swapchainPresentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapchain;

        TextureUsage textureUsage = TextureUsage::RenderTarget;

        // Enable transfer source on swap chain images if supported.
        if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        // Enable transfer destination on swap chain images if supported
        if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        VkResult result = vkCreateSwapchainKHR(_device->GetImpl()->GetVkDevice(), &createInfo, nullptr, &_handle);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan: Failed to create swapchain, error: {}", GetVkResultString(result));
            return false;
        }


        if (oldSwapchain != VK_NULL_HANDLE)
        {
            for (uint32_t i = 0u; i < _imageCount; ++i)
            {
                vkDestroySemaphore(_device->GetImpl()->GetVkDevice(), _imageSemaphores[i], nullptr);
                //_swapchainTextures[i].reset(nullptr);
            }

            vkDestroySwapchainKHR(_device->GetImpl()->GetVkDevice(), oldSwapchain, nullptr);
        }

        _imageIndex = _semaphoreIndex = 0;
        vkGetSwapchainImagesKHR(_device->GetImpl()->GetVkDevice(), _handle, &_imageCount, nullptr);
        _images.resize(_imageCount);
        _imageSemaphores.resize(_imageCount);
        //_swapchainTextures.resize(_imageCount);
        vkGetSwapchainImagesKHR(_device->GetImpl()->GetVkDevice(), _handle, &_imageCount, _images.data());

        _width = swapchainSize.width;
        _height = swapchainSize.height;
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM) {
            _colorFormat = PixelFormat::BGRA8UNorm;
        }
        else if (format.format == VK_FORMAT_B8G8R8A8_SRGB) {
            _colorFormat = PixelFormat::BGRA8UNormSrgb;
        }
        else if (format.format == VK_FORMAT_R8G8B8A8_UNORM) {
            _colorFormat = PixelFormat::RGBA8UNorm;
        }
        else if (format.format == VK_FORMAT_R8G8B8A8_SRGB) {
            _colorFormat = PixelFormat::RGBA8UNormSrgb;
        }

        // Create render pass
        //CreateRenderPass();

        // Create command buffer for transition or clear 
        auto setupSwapchainCmdBuffer = _device->GetCommandQueue(QueueType::Direct)->GetCommandBuffer();

        for (uint32_t i = 0u; i < _imageCount; ++i)
        {
            VkSemaphoreCreateInfo semaphoreCreateInfo = {};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            vkThrowIfFailed(vkCreateSemaphore(_device->GetImpl()->GetVkDevice(), &semaphoreCreateInfo, NULL, &_imageSemaphores[i]));

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
                    setupSwapchainCmdBuffer->GetHandle(),
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
                    setupSwapchainCmdBuffer->GetHandle(),
                    _images[i],
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                );
            }

            // Create backend texture.
            TextureDescriptor textureDescriptor = {};
            textureDescriptor.width = _width;
            textureDescriptor.height = _height;
            textureDescriptor.depth = 1;
            textureDescriptor.arraySize = 1;
            textureDescriptor.mipLevels = 1;
            textureDescriptor.samples = SampleCount::Count1;
            textureDescriptor.type = TextureType::Type2D;
            textureDescriptor.format = _colorFormat;
            textureDescriptor.usage = textureUsage;
            //_swapchainTextures[i].reset(new TextureVk(_device, &textureDescriptor, _images[i], nullptr));

            // Create backend framebuffer.
            //FramebufferDescriptor fboDescriptor = {};
            //fboDescriptor.colorAttachments[0].texture = _swapchainTextures[i].get();;
            //_framebuffers[i] = new FramebufferVk(_device, _renderPass, &fboDescriptor);
        }

        _device->GetCommandQueue(QueueType::Direct)->Submit(setupSwapchainCmdBuffer, true);

        return true;
    }

    void SwapChainVk::CreateRenderPass()
    {
        if (_renderPass != VK_NULL_HANDLE) {
            //vkDestroyRenderPass(_device->GetVkDevice(), _renderPass, nullptr);
        }

        /*VkAttachmentDescription attachments[4u] = {};
        uint32_t attachmentCount = 0u;
        VkAttachmentReference colorReference;
        VkAttachmentReference resolveReference;
        VkAttachmentReference depthReference;

        VkSubpassDescription subpassDescription;
        subpassDescription.flags = 0u;
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        subpassDescription.pResolveAttachments = nullptr;
        subpassDescription.pDepthStencilAttachment = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;

        // Subpass dependencies for layout transitions
        VkSubpassDependency dependencies[2u];

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0u;
        createInfo.attachmentCount = attachmentCount;
        createInfo.pAttachments = attachments;
        createInfo.subpassCount = 1u;
        createInfo.pSubpasses = &subpassDescription;
        createInfo.dependencyCount = 2u;
        createInfo.pDependencies = dependencies;
        VkResult result = vkCreateRenderPass(_device->GetVkDevice(), &createInfo, nullptr, &_renderPass);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan: Failed to create render pass: '{}'", GetVkResultString(result));
        }
        else
        {
            ALIMER_LOGDEBUG("Vulkan: Created render pass with success");
        }*/
    }

    void SwapChainVk::AcquireNextTexture()
    {
        VkSemaphore semaphore = _imageSemaphores[_semaphoreIndex];
        _semaphoreIndex = (_semaphoreIndex + 1) % _imageCount;

        VkResult result = vkAcquireNextImageKHR(
            _device->GetVkDevice(),
            _handle, UINT64_MAX,
            semaphore,
            VK_NULL_HANDLE,
            &_imageIndex);

        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
            // Resize();
        }
        else {
            vkThrowIfFailed(result);
        }
    }

    VkResult SwapChainVk::QueuePresent(VkQueue queue)
    {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = NULL;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &_handle;
        presentInfo.pImageIndices = &_imageIndex;
        /*
        // Check if a wait semaphore has been specified to wait for before presenting the image
        if (waitSemaphore != VK_NULL_HANDLE)
        {
            presentInfo.pWaitSemaphores = &waitSemaphore;
            presentInfo.waitSemaphoreCount = 1;
        }*/

        return vkQueuePresentKHR(queue, &presentInfo);
    }
}

#endif // TODO_VK
