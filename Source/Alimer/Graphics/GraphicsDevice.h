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

#include "../Core/Object.h"
#include "../Application/Window.h"
#include "../Graphics/Types.h"
#include "../Graphics/GpuDeviceFeatures.h"
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Shader.h"
#include "../Graphics/CommandBuffer.h"
#include "../Graphics/VertexFormat.h"
#include <vector>

#ifdef ALIMER_THREADING
#include <atomic>
#include <mutex>
#include <condition_variable>
#endif

namespace Alimer
{
    /// Enum describing the Graphics backend.
    enum class GraphicsBackend : uint32_t
    {
        /// Best device supported for running platform.
        Default,
        /// Empty/Headless device type.
        Empty,
        /// Vulkan backend.
        Vulkan,
        /// DirectX 11.1+ backend.
        Direct3D11,
        /// DirectX 12 backend.
        Direct3D12,
    };

    class ALIMER_API RenderingSettings
    {
    public:
        // Main swap chain settings.
        uint32_t defaultBackBufferWidth = 1280;
        uint32_t defaultBackBufferHeight = 720;
        WindowHandle windowHandle = 0;
    };

    /// Low-level 3D graphics module.
    class ALIMER_API Graphics : public Object
    {
        friend class GpuResource;

        ALIMER_OBJECT(Graphics, Object);

    protected:
        /// Constructor.
        Graphics(bool validation);

    public:
        static Graphics* Create(bool validation = false);

        /// Destructor.
        ~Graphics() override;

        /// Initialize graphics with given settings.
        virtual bool Initialize(const RenderingSettings& settings);

        /// Wait for a device to become idle.
        bool WaitIdle();

        /// Begin the rendering frame.
        virtual bool BeginFrame() = 0;

        /// Finishes the current frame and schedules it for display.
        virtual void EndFrame() = 0;

        /*
        
        /// Create new buffer with given descriptor and optional initial data.
        GpuBuffer* CreateBuffer(const BufferDescriptor* descriptor, const void* initialData = nullptr);

        /// Create new VertexInputFormat with given descriptor.
        VertexInputFormat* CreateVertexInputFormat(const VertexInputFormatDescriptor* descriptor);

        /// Create new shader module using SPIRV bytecode.
        ShaderModule* CreateShaderModule(const std::vector<uint32_t>& spirv);

        /// Create new shader module from file and given entry point.
        ShaderModule* CreateShaderModule(const String& file, const String& entryPoint = "main");

        /// Create new shader program with descriptor.
        ShaderProgram* CreateShaderProgram(const ShaderProgramDescriptor* descriptor);

        /// Create new shader compute shader with descriptor.
        ShaderProgram* CreateShaderProgram(const ShaderStageDescriptor* stage);

        /// Create new shader program with vertex and fragment module.
        ShaderProgram* CreateShaderProgram(ShaderModule* vertex, ShaderModule* fragment);
        */

        /// Get whether grapics has been initialized.
        bool IsInitialized() const { return _initialized; }

        /// Get the type of device.
        GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device features.
        const GraphicsDeviceFeatures& GetFeatures() const { return _features; }

        /// Get the main Swapchain current image view.
        virtual SharedPtr<TextureView> GetSwapchainView() const = 0;

        /// Get the main command buffer.
        SharedPtr<CommandBuffer> GetMainCommandBuffer() const
        {
            return _mainCommandBuffer;
        }

        /// Request new command buffer.
        //SharedPtr<CommandBuffer> RequestCommandBuffer(CommandBuffer::Type type);

        uint64_t GetNextUniqueId();

        void NotifyFalidationError(const char* message);

        // Backend
#if ALIMER_VULKAN
        VkInstance GetInstance() const { return _instance; }
        VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
        VkDevice GetDevice() const { return _device; }
        VmaAllocator_T* GetVmaAllocator() const { return _allocator; }
#endif

    private:
        void InitializeBackend();
        void ShutdownBackend();

    protected:
        /// Add a GpuResource to keep track of. 
        void AddGpuResource(GpuResource* resource);

        /// Remove a GpuResource.
        void RemoveGpuResource(GpuResource* resource);
        //virtual GpuBuffer* CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData) = 0;
        //virtual VertexInputFormat* CreateVertexInputFormatImpl(const VertexInputFormatDescriptor* descriptor) = 0;
        //virtual ShaderModule* CreateShaderModuleImpl(const std::vector<uint32_t>& spirv) = 0;
        //virtual ShaderProgram* CreateShaderProgramImpl(const ShaderProgramDescriptor* descriptor) = 0;
        //virtual Texture* CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData = nullptr) = 0;

        GraphicsBackend _backend = GraphicsBackend::Empty;
        bool _validation = false;

        bool _initialized = false;
        RenderingSettings _settings{};
        GraphicsDeviceFeatures _features{};

        SharedPtr<CommandBuffer> _mainCommandBuffer;
        std::vector<GpuResource*> _gpuResources;

#if ALIMER_VULKAN
        bool _supportsExternal = false;
        bool _supportsDedicated = false;
        bool _supportsImageFormatList = false;
        bool _supportsDebugMarker = false;
        bool _supportsDebugUtils = false;

        VkInstance _instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT _debugCallback = VK_NULL_HANDLE;

        // PhysicalDevice
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties _deviceProperties;
        VkPhysicalDeviceMemoryProperties _deviceMemoryProperties;
        VkPhysicalDeviceFeatures _deviceFeatures;
        std::vector<VkQueueFamilyProperties> _queueFamilyProperties;

        // Logical device.
        VkDevice _device = nullptr;
        VmaAllocator_T* _allocator = nullptr;

        // Queue's.
        VkQueue _graphicsQueue = VK_NULL_HANDLE;
        VkQueue _computeQueue = VK_NULL_HANDLE;
        VkQueue _transferQueue = VK_NULL_HANDLE;
        uint32_t _graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t _computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t _transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;

        // Frame fences
        std::vector<VkFence> _waitFences;
        uint32_t _swapchainImageIndex = 0;
        VkSemaphore _swapchainImageAcquiredSemaphore = VK_NULL_HANDLE;
#endif

    private:
#ifdef ALIMER_THREADING
        std::atomic<uint64_t> _cookie;
        std::mutex _gpuResourceMutex;
#else
        uint64_t _cookie = 0;
#endif
    };
}
