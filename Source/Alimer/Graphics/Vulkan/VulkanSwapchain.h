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

#include "../../Application/Window.h"
#include "VulkanPrerequisites.h"
#include <vector>

namespace Alimer
{
	class VulkanTexture;
    class VulkanRenderPass;
	class VulkanGraphics;

	/// Vulkan Swapchain.
	class VulkanSwapchain final
	{
	public:
		/// Construct. Set parent shader and defines but do not compile yet.
		VulkanSwapchain(VulkanGraphics* graphics, Window* window);
		/// Destruct.
		~VulkanSwapchain();

		void Resize(uint32_t width, uint32_t height, bool force = false);

		SharedPtr<VulkanRenderPass> GetNextDrawable();
        VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex);
		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);


		uint32_t GetImageCount() const { return _imageCount; }
        VulkanRenderPass* GetRenderPass(uint32_t index) const { return _renderPasses[index].Get(); }

	private:
		/// Graphics subsystem.
		WeakPtr<VulkanGraphics> _graphics;

		VkInstance _instance;
		VkPhysicalDevice _physicalDevice;
		VkDevice _logicalDevice;

		VkSurfaceKHR _surface;
		VkSurfaceFormatKHR _swapchainFormat{};
		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
		
		std::vector<VkImage> _images;
        std::vector<SharedPtr<VulkanTexture>> _textures;
		std::vector<SharedPtr<VulkanRenderPass>> _renderPasses;
        std::vector<VkSemaphore> _semaphores;

        uint32_t _imageCount;
        uint32_t _currentSemaphoreIndex;
        uint32_t _currentBackBufferIndex;

		uint32_t _width{};
		uint32_t _height{};
		bool _vsync{ false };
	};
}
