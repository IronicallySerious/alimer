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
#include "VulkanTexture.h"
#include "VulkanRenderPass.h"
#include "VulkanConvert.h"
#include "../../Core/Log.h"

namespace Alimer
{
	VulkanSwapchain::VulkanSwapchain(VulkanGraphics* graphics, void* windowHandle, const uvec2& size)
		: _graphics(graphics)
        , _size(size)
		, _instance(graphics->GetInstance())
		, _physicalDevice(graphics->GetPhysicalDevice())
		, _logicalDevice(graphics->GetLogicalDevice())
        , _imageCount(0)
        , _currentSemaphoreIndex(0)
        , _currentBackBufferIndex(0)
	{
		VkResult result = VK_SUCCESS;

		// Create the os-specific surface.
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
		surfaceCreateInfo.hinstance = GetModuleHandleW(nullptr);
		surfaceCreateInfo.hwnd = static_cast<HWND>(windowHandle);
		result = vkCreateWin32SurfaceKHR(_instance, &surfaceCreateInfo, nullptr, &_surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.display = display;
        surfaceCreateInfo.surface = window;
        err = vkCreateWaylandSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.connection = connection;
        surfaceCreateInfo.window = window;
        err = vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = { VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR };
		surfaceCreateInfo.window = static_cast<ANativeWindow*>(windowHandle);
		result = vkCreateAndroidSurfaceKHR(_instance, &surfaceCreateInfo, nullptr, &_surface);
#endif
		vkThrowIfFailed(result);

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
		assert(queueFamilyCount > 0);
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

		std::vector<VkBool32> supportsPresent(queueFamilyCount);
		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &supportsPresent[i]);
		}

		// Search for a graphics and a present queue in the array of queue
		// families, try to find one that supports both
		uint32_t graphicsQueueNodeIndex = UINT32_MAX;
		uint32_t presentQueueNodeIndex = UINT32_MAX;
		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{
			if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (graphicsQueueNodeIndex == UINT32_MAX)
				{
					graphicsQueueNodeIndex = i;
				}

				if (supportsPresent[i] == VK_TRUE)
				{
					graphicsQueueNodeIndex = i;
					presentQueueNodeIndex = i;
					break;
				}
			}
		}
		if (presentQueueNodeIndex == UINT32_MAX)
		{
			// If there's no queue that supports both present and graphics
			// try to find a separate present queue
			for (uint32_t i = 0; i < queueFamilyCount; ++i)
			{
				if (supportsPresent[i] == VK_TRUE)
				{
					presentQueueNodeIndex = i;
					break;
				}
			}
		}

		// Exit if either a graphics or a presenting queue hasn't been found
		if (graphicsQueueNodeIndex == UINT32_MAX
			|| presentQueueNodeIndex == UINT32_MAX)
		{
			ALIMER_LOGCRITICAL("Could not find a graphics and/or presenting queue");
			return;
		}

		// Get list of supported surface formats
		uint32_t formatCount;
		vkThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, NULL));
		assert(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, surfaceFormats.data()));

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
            vkDestroySemaphore(_logicalDevice, _semaphores[i], nullptr);
            //vkDestroyImageView(_logicalDevice, buffers[i].view, nullptr);
        }

        _semaphores.clear();

		if (_swapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(_logicalDevice, _swapchain, nullptr);
		}

		if (_surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(_instance, _surface, nullptr);
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
                vkDestroySemaphore(_logicalDevice, _semaphores[i], nullptr);
				//vkDestroyImageView(_logicalDevice, buffers[i].view, nullptr);
			}

			vkDestroySwapchainKHR(_logicalDevice, oldSwapchain, nullptr);
            _semaphores.clear();
		}
		vkThrowIfFailed(vkGetSwapchainImagesKHR(_logicalDevice, _swapchain, &_imageCount, nullptr));

		// Get the swap chain images
		_images.resize(_imageCount);
        _textures.resize(_imageCount);
        _renderPasses.resize(_imageCount);
		vkThrowIfFailed(vkGetSwapchainImagesKHR(_logicalDevice, _swapchain, &_imageCount, _images.data()));

        // Create the semaphores
        VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        _semaphores.resize(_imageCount, VK_NULL_HANDLE);
        _currentSemaphoreIndex = 0;
        _currentBackBufferIndex = 0;

		TextureDescriptor textureDesc = {};
		textureDesc.type = TextureType::Type2D;
		textureDesc.usage = TextureUsage::RenderTarget;
		textureDesc.format = _format;
		textureDesc.width = _size.x;
		textureDesc.height = _size.y;

		for (uint32_t i = 0; i < _imageCount; i++)
		{
            vkCreateSemaphore(_logicalDevice, &semaphoreCreateInfo, nullptr, &_semaphores[i]);

			_textures[i] = MakeUnique<VulkanTexture>(
				_graphics,
				&textureDesc,
				_images[i],
				createInfo.imageUsage);

            RenderPassDescription passDescription = {};
            passDescription.colorAttachments[0].texture = _textures[i].Get();
            passDescription.colorAttachments[0].loadAction = LoadAction::Clear;
            passDescription.colorAttachments[0].storeAction = StoreAction::Store;
            _renderPasses[i] = new VulkanRenderPass(_graphics, &passDescription);
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
					_images[i],
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

	/*SharedPtr<VulkanRenderPass> VulkanSwapchain::GetNextDrawable()
	{
        VkSemaphore semaphore = _semaphores[_currentSemaphoreIndex];
        _currentSemaphoreIndex = (_currentSemaphoreIndex + 1) % _imageCount;

		VkResult result =  vkAcquireNextImageKHR(
			_logicalDevice,
			_swapchain,
			UINT64_MAX,
            semaphore,
			(VkFence)nullptr,
            &_currentBackBufferIndex);

		if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
        {
			Resize(_width, _height, true);
			return GetNextDrawable();
		}
		else {
			vkThrowIfFailed(result);
		}

        _graphics->AddWaitSemaphore(semaphore);
		return _renderPasses[_currentBackBufferIndex];
	}*/

    bool VulkanSwapchain::AcquireNextTexture(uint32_t* textureIndex)
    {
        VkResult result = vkAcquireNextImageKHR(
            _logicalDevice,
            _swapchain,
            UINT64_MAX,
            (VkSemaphore)nullptr,
            (VkFence)nullptr,
            textureIndex);

        return result == VK_SUCCESS;
    }

    void VulkanSwapchain::Present()
    {

    }

    VkResult VulkanSwapchain::AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex)
    {
        return vkAcquireNextImageKHR(_logicalDevice, _swapchain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, imageIndex);
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
