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

#include "../Graphics.h"
#include "../../Util/HashMap.h"
#include "VulkanSwapchain.h"

namespace Alimer
{
	class VulkanCommandBuffer;

	struct VulkanFramebuffer
	{
		VkRenderPass renderPass;
		VkFramebuffer framebuffer;
		VkExtent2D size;
	};

	/// Vulkan graphics backend.
	class VulkanGraphics final : public Graphics
	{
	public:
        static bool IsSupported();

		/// Construct. Set parent shader and defines but do not compile yet.
		VulkanGraphics(bool validation, const std::string& applicationName);
		/// Destruct.
		~VulkanGraphics() override;

        bool WaitIdle() override;
        bool Initialize(const SharedPtr<Window>& window) override;
		SharedPtr<Texture> AcquireNextImage() override;
		bool Present() override;

		SharedPtr<CommandBuffer> GetCommandBuffer() override;

        SharedPtr<GpuBuffer> CreateBuffer(BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData) override;
		SharedPtr<Shader> CreateComputeShader(const ShaderStageDescription& desc) override;
		SharedPtr<Shader> CreateShader(const ShaderStageDescription& vertex, const ShaderStageDescription& fragment) override;
        SharedPtr<PipelineState> CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor) override;

		VkInstance GetInstance() const { return _instance; }
		VkPhysicalDevice GetPhysicalDevice() const { return _vkPhysicalDevice; }
		VkDevice GetLogicalDevice() const { return _logicalDevice; }
        VmaAllocator GetAllocator() const { return _allocator; }

		VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
		void ClearImageWithColor(VkCommandBuffer commandBuffer, VkImage image, VkImageSubresourceRange range, VkImageAspectFlags aspect, VkImageLayout sourceLayout, VkImageLayout destLayout, VkAccessFlagBits srcAccessMask, VkClearColorValue *clearValue);
		void SubmitCommandBuffer(VulkanCommandBuffer* commandBuffer);

		VkRenderPass GetVkRenderPass(const RenderPassDescriptor& descriptor, uint64_t hash);
		VulkanFramebuffer* GetFramebuffer(VkRenderPass renderPass, const RenderPassDescriptor& descriptor, uint64_t hash);
        uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags);
        VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags);

	private:
        void Finalize() override;
        void CreateAllocator();
		bool PrepareDraw(PrimitiveTopology topology);

		VkInstance _instance = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT _debugCallback = VK_NULL_HANDLE;

		// PhysicalDevice
		VkPhysicalDevice _vkPhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties _deviceProperties;
		VkPhysicalDeviceFeatures _deviceFeatures;
		VkPhysicalDeviceMemoryProperties _deviceMemoryProperties;
		std::vector<VkQueueFamilyProperties> _queueFamilyProperties;

		// LogicalDevice
		struct {
			uint32_t graphics;
			uint32_t compute;
		} _queueFamilyIndices;

		VkDevice _logicalDevice = VK_NULL_HANDLE;
        VmaAllocator _allocator = VK_NULL_HANDLE;

		// Queue's.
		VkQueue _graphicsQueue = VK_NULL_HANDLE;
		VkQueue _computeQueue = VK_NULL_HANDLE;

		UniquePtr<VulkanSwapchain> _swapchain;
		uint32_t _swapchainImageIndex = 0;

		// CommandPools
		VkCommandPool _commandPool = VK_NULL_HANDLE;
		std::vector<SharedPtr<VulkanCommandBuffer>> _commandBuffers;

		// Sync semaphores
		VkSemaphore _imageAcquiredSemaphore = VK_NULL_HANDLE;
		VkSemaphore	_renderCompleteSemaphore = VK_NULL_HANDLE;
		std::vector<VkFence> _waitFences;

		// Cache
		std::unordered_map<uint64_t, VkRenderPass> _renderPassCache;
		std::unordered_map<uint64_t, VulkanFramebuffer*> _framebufferCache;
	};
}
