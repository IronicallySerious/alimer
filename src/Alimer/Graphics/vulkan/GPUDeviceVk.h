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
    class CommandContextVk;

    /// Vulkan gpu backend.
    class GraphicsDeviceVk final : public GraphicsDevice
    {
    public:
        /// Is backend supported?
        static bool IsSupported();

        /// Constructor.
        GraphicsDeviceVk(const char* applicationName, PhysicalDevicePreference devicePreference, bool validation);

        /// Destructor.
        ~GraphicsDeviceVk() override;

        void Finalize() override;
        void WaitIdle();
        
        void CreateInstance(const char* applicationName);
        void SelectPhysicalDevice(PhysicalDevicePreference devicePreference);
        void CreateLogicalDevice(VkSurfaceKHR surface);
        void InitializeFeatures();

        bool InitializeImpl(const SwapChainDescriptor* descriptor) override;
        bool BeginFrameImpl() override;
        void EndFrame(uint32_t frameId) override;

        VkSurfaceKHR CreateSurface(const SwapChainDescriptor* descriptor);

        SharedPtr<CommandContext> GetContext() const override;

        bool ImageFormatIsSupported(VkFormat format, VkFormatFeatureFlags required, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL) const;
        PixelFormat GetDefaultDepthStencilFormat() const;
        PixelFormat GetDefaultDepthFormat() const;

        VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);

        VkSemaphore RequestSemaphore();
        void RecycleSemaphore(VkSemaphore semaphore);

        VkFence RequestFence();
        void RecycleFence(VkFence fence);

        void DestroySampler(VkSampler handle);
        void DestroyPipeline(VkPipeline handle);
        void DestroyImage(VkImage handle);
        
        VkInstance GetVkInstance() const { return _instance; }
        VkPhysicalDevice GetVkPhysicalDevice() const { return _physicalDevice; }
        VkDevice GetVkDevice() const { return _device; }
        VmaAllocator GetVmaAllocator() const { return _memoryAllocator; }

        uint32_t GetGraphicsQueueFamily() const { return _graphicsQueueFamily; }
        uint32_t GetComputeQueueFamily() const { return _computeQueueFamily; }
        uint32_t GetTransferQueueFamily() const { return _transferQueueFamily; }
        const DeviceFeaturesVk& GetFeaturesVk() const { return _featuresVk; }

    private:
        bool HasExtension(const std::string& extension)
        {
            return (std::find(_physicalDeviceExtensions.begin(), _physicalDeviceExtensions.end(), extension) != _physicalDeviceExtensions.end());
        }

        bool HasLayer(const std::string& extension)
        {
            return (std::find(_physicalDeviceLayers.begin(), _physicalDeviceLayers.end(), extension) != _physicalDeviceLayers.end());
        }

    private:
        bool                                    _headless;
        VkInstance                              _instance = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT                _debugCallback = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT                _debugMessenger = VK_NULL_HANDLE;
        VkPhysicalDevice                        _physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties              _physicalDeviceProperties;
        VkPhysicalDeviceMemoryProperties        _physicalDeviceMemoryProperties;
        VkPhysicalDeviceFeatures                _physicalDeviceFeatures;
        std::vector<VkQueueFamilyProperties>    _physicalDeviceQueueFamilyProperties;
        std::vector<std::string>                _physicalDeviceExtensions;
        std::vector<std::string>                _physicalDeviceLayers;
        uint32_t                                _graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t                                _computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t                                _transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        VkDevice                                _device = VK_NULL_HANDLE;
        VkQueue                                 _graphicsQueue = VK_NULL_HANDLE;
        VkQueue                                 _computeQueue = VK_NULL_HANDLE;
        VkQueue                                 _transferQueue = VK_NULL_HANDLE;
        VmaAllocator                            _memoryAllocator = VK_NULL_HANDLE;
        DeviceFeaturesVk                        _featuresVk = {};
        SwapChainVk*                            _swapChain = nullptr;
        std::vector<VkFence>                    _waitFences;
        std::vector<VkSemaphore>                _renderCompleteSemaphores;
        std::vector<VkSemaphore>                _presentCompleteSemaphores;
        uint32_t                                _frameIndex = 0u;
        uint32_t                                _maxInflightFrames = 0u;
        uint32_t                                _swapchainImageIndex = 0u;
        VkCommandPool                           _graphicsCommandPool = VK_NULL_HANDLE;
        VkPipelineCache                         _pipelineCache = VK_NULL_HANDLE;
        std::vector<SharedPtr<CommandContextVk>> _commandBuffers;
        std::vector<VkFence>                    _fences;
        std::vector<VkSemaphore>                _semaphores;
        

        /* Per Frame data */
        /*struct FrameData
        {
            FrameData(GraphicsDeviceVk* device_);
            ~FrameData();
            void operator=(const FrameData&) = delete;
            FrameData(const FrameData&) = delete;

            void ProcessDeferredDelete();

            GraphicsDeviceVk* device;
            VkDevice logicalDevice;
            std::vector<VkCommandBuffer>    submittedCmdBuffers;
            std::vector<VkSemaphore>        waitSemaphores;
            VkFence                         fence;
            std::vector<VkSampler>          destroyedSamplers;
            std::vector<VkPipeline>         destroyedPipelines;
            std::vector<VkImage>            destroyedImages;
        };

        std::vector<std::unique_ptr<FrameData>> _frameData;

        FrameData &frame()
        {
            ALIMER_ASSERT(_frameIndex < _frameData.size());
            ALIMER_ASSERT(_frameData[_frameIndex]);
            return *_frameData[_frameIndex];
        }

        const FrameData &frame() const
        {
            ALIMER_ASSERT(_frameIndex < _frameData.size());
            ALIMER_ASSERT(_frameData[_frameIndex]);
            return *_frameData[_frameIndex];
        }

        struct QueueData
        {
            std::vector<VkSemaphore> waitSemaphores;
            std::vector<VkPipelineStageFlags> waitStages;
        } _graphics, _compute, _transfer;*/
    };
}
