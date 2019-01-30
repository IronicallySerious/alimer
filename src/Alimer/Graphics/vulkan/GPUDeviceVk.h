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

#include "BackendVk.h"
#include "../GraphicsDevice.h"

namespace alimer
{
    class SwapChainVk;
    class CommandQueueVk;
    class CommandBufferVk;

    /// Vulkan gpu backend.
    class GPUDeviceVk final : public GraphicsDevice
    {
    public:
        // Constants
        static constexpr uint32_t RenderLatency = 2u;

        /// Is backend supported?
        static bool IsSupported();

        /// Constructor.
        GPUDeviceVk(bool validation, bool headless);

        /// Destructor.
        ~GPUDeviceVk() override;

        /// Wait device idle.
        void WaitIdle() override;

        //bool SetMode(const SwapChainHandle* handle, const SwapChainDescriptor* descriptor) override;
        //bool BeginFrame() override;
        //void EndFrame() override;

        //GPUTexture* CreateTexture(const TextureDescriptor* descriptor, void* nativeTexture, const void* pInitData) override;
        //GPUSampler* CreateSampler(const SamplerDescriptor* descriptor) override;
        //GPUBuffer* CreateBuffer(const BufferDescriptor* descriptor, const void* pInitData) override;

        void DestroySampler(VkSampler sampler);

        VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
        void ClearImageWithColor(VkCommandBuffer commandBuffer, VkImage image, VkImageSubresourceRange range, VkImageAspectFlags aspect, VkImageLayout sourceLayout, VkImageLayout destLayout, VkClearColorValue *clearValue);

        /// notify when validation layers report error.
        void NotifyValidationError(const char* message);

        VkInstance GetVkInstance() const { return _instance; }
        VkPhysicalDevice GetVkPhysicalDevice() const { return _physicalDevice; }
        VkDevice GetVkDevice() const { return _device; }
        VmaAllocator GetVmaAllocator() const { return _memoryAllocator; }

        uint32_t GetGraphicsQueueFamily() const { return _graphicsQueueFamily; }
        uint32_t GetComputeQueueFamily() const { return _computeQueueFamily; }
        uint32_t GetTransferQueueFamily() const { return _transferQueueFamily; }
        const DeviceFeaturesVk& GetFeaturesVk() const { return _featuresVk; }

    private:
        void InitializeFeatures();
        //VkSurfaceKHR CreateSurface(const SwapChainHandle* handle);
        bool IsExtensionSupported(const String& extension)
        {
            return (std::find(_physicalDeviceExtensions.begin(), _physicalDeviceExtensions.end(), extension) != _physicalDeviceExtensions.end());
        }

    private:
        VkInstance                              _instance = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT                _debugCallback = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT                _debugMessenger = VK_NULL_HANDLE;
        VkPhysicalDevice                        _physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties              _physicalDeviceProperties;
        VkPhysicalDeviceMemoryProperties        _physicalDeviceMemoryProperties;
        VkPhysicalDeviceFeatures                _physicalDeviceFeatures;
        std::vector<VkQueueFamilyProperties>    _physicalDeviceQueueFamilyProperties;
        std::vector<String>                     _physicalDeviceExtensions;
        uint32_t                                _graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t                                _computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t                                _transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        VkDevice                                _device = VK_NULL_HANDLE;
        VkQueue                                 _graphicsQueue = VK_NULL_HANDLE;
        VkQueue                                 _computeQueue = VK_NULL_HANDLE;
        VkQueue                                 _transferQueue = VK_NULL_HANDLE;
        VmaAllocator                            _memoryAllocator = VK_NULL_HANDLE;
        std::unique_ptr<CommandQueueVk>         _graphicsCommandQueue;
        std::unique_ptr<CommandQueueVk>         _computeCommandQueue;
        std::unique_ptr<CommandQueueVk>         _copyCommandQueue;
        std::vector<std::unique_ptr<CommandBufferVk>> _commandBuffers;

        /* Features */
        DeviceFeaturesVk                        _featuresVk;
        SwapChainVk*                             _mainSwapChain = nullptr;
        std::vector<VkFence>                    _waitFences;
        std::vector<VkSemaphore>                _renderCompleteSemaphores;
        std::vector<VkSemaphore>                _presentCompleteSemaphores;
        uint32_t                                _frameIndex = 0;
        uint32_t                                _swapchainImageIndex = 0;

        /* Per Frame data */
        struct PerFrame
        {
            PerFrame(GPUDeviceVk* device_);
            ~PerFrame();
            void operator=(const PerFrame &) = delete;
            PerFrame(const PerFrame &) = delete;

            void Begin();

            GPUDeviceVk* device;
            VkDevice logicalDevice;
            std::vector<VkSampler> destroyedSamplers;
        };

        std::vector<std::unique_ptr<PerFrame>> _perFrame;

        PerFrame &frame()
        {
            ALIMER_ASSERT(_frameIndex < _perFrame.size());
            ALIMER_ASSERT(_perFrame[_frameIndex]);
            return *_perFrame[_frameIndex];
        }

        const PerFrame &frame() const
        {
            ALIMER_ASSERT(_frameIndex < _perFrame.size());
            ALIMER_ASSERT(_perFrame[_frameIndex]);
            return *_perFrame[_frameIndex];
        }

    };
}
