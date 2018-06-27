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
    class VulkanCommandQueue;
	class VulkanCommandBuffer;
    class VulkanDescriptorSetAllocator;
    class VulkanPipelineLayout;

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

        void WaitIdle() override;
       
		SharedPtr<Texture> AcquireNextImage() override;
        CommandBuffer* GetDefaultCommandBuffer() const override;

        SharedPtr<GpuBuffer> CreateBuffer(const GpuBufferDescription& description, const void* initialData) override;
		SharedPtr<Shader> CreateComputeShader(const ShaderStageDescription& desc) override;
		SharedPtr<Shader> CreateShader(const ShaderStageDescription& vertex, const ShaderStageDescription& fragment) override;
        SharedPtr<PipelineState> CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor) override;

		VkInstance GetInstance() const { return _instance; }
		VkPhysicalDevice GetPhysicalDevice() const { return _vkPhysicalDevice; }
		VkDevice GetLogicalDevice() const { return _logicalDevice; }
        VmaAllocator GetAllocator() const { return _allocator; }
        VkPipelineCache GetPipelineCache() const { return _pipelineCache; }

		VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
		void ClearImageWithColor(VkCommandBuffer commandBuffer, VkImage image, VkImageSubresourceRange range, VkImageAspectFlags aspect, VkImageLayout sourceLayout, VkImageLayout destLayout, VkAccessFlagBits srcAccessMask, VkClearColorValue *clearValue);

		VkRenderPass GetVkRenderPass(const RenderPassDescriptor& descriptor, uint64_t hash);
		VulkanFramebuffer* GetFramebuffer(VkRenderPass renderPass, const RenderPassDescriptor& descriptor, uint64_t hash);

        VulkanDescriptorSetAllocator* RequestDescriptorSetAllocator(const DescriptorSetLayout &layout);
        VulkanPipelineLayout* RequestPipelineLayout(const ResourceLayout &layout);

        uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags);
        VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags);

        void AddWaitSemaphore(VkSemaphore semaphore);

	private:
        void Finalize() override;
        bool BackendInitialize() override;
        bool BeginFrameCore() override;
        void EndFrameCore() override;
        void CreateAllocator();

		VkInstance _instance = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT _debugCallback = VK_NULL_HANDLE;

		// PhysicalDevice
		VkPhysicalDevice _vkPhysicalDevice = VK_NULL_HANDLE;
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

        VkPipelineCache _pipelineCache = VK_NULL_HANDLE;

		// CommandPools
        SharedPtr<VulkanCommandQueue> _defaultCommandQueue;
        SharedPtr<CommandBuffer> _defaultCommandBuffer;
		VkCommandPool _setupCommandPool = VK_NULL_HANDLE;

        std::vector<VkSemaphore> _waitSemaphores;
        uint64_t _frameIndex = 1;

		// Cache
		std::unordered_map<uint64_t, VkRenderPass> _renderPassCache;
		std::unordered_map<uint64_t, VulkanFramebuffer*> _framebufferCache;

        HashMap<std::unique_ptr<VulkanDescriptorSetAllocator>> _descriptorSetAllocators;
        HashMap<std::unique_ptr<VulkanPipelineLayout>> _pipelineLayouts;
	};
}
