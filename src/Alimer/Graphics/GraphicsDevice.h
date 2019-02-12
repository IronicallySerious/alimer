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

#pragma once

#include "../Core/Object.h"
#include "../Graphics/GraphicsDeviceFeatures.h"
#include "../Graphics/CommandBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Pipeline.h"
#include <memory>
#include <vector>
#include <queue>
#include <mutex>

namespace alimer
{
    class Shader;
    class Buffer;
    class Sampler;
    class Framebuffer;

    struct GraphicsDeviceDescriptor {
        GraphicsBackend             preferredBackend = GraphicsBackend::Default;
        PhysicalDevicePreference    devicePreference = PhysicalDevicePreference::Discrete;
        bool                        validation = false;
        /// Main swap chain descriptor or null for headless.
        const SwapChainDescriptor*  swapchain = nullptr;
    };

    /// Low-level graphics module.
    class ALIMER_API GraphicsDevice : public Object
    {
        friend class GPUResource;
        friend class CommandContext;

        ALIMER_OBJECT(GraphicsDevice, Object);

    public:
        /// Constructor.
        static GraphicsDevice* Create(
            GraphicsBackend preferredBackend = GraphicsBackend::Default,
            PhysicalDevicePreference devicePreference = PhysicalDevicePreference::Discrete);

        /// Destructor.
        virtual ~GraphicsDevice() override;

        /// Initialize device with main swap chain descriptor.
        bool Initialize(const SwapChainDescriptor* descriptor);

        /// Begin rendering frame.
        bool BeginFrame();

        /// Finishes the current frame and advances to next one.
        uint64_t Frame();

        /// Wait device to finish all pending operations.
        void WaitIdle();

        /// Get the default framebuffer.
        virtual Framebuffer* GetDefaultFramebuffer() const = 0;

        /// Get the backend.
        inline GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device features.
        inline const GraphicsDeviceFeatures& GetFeatures() const { return _features; }

        /**
        * Create a 1D texture.
        *
        * @param width The width of the texture.
        * @param format The format of the texture.
        * @param arraySize The array size of the texture.
        * @param mipLevels if equal to 0 then an entire mip chain will be generated from mip level 0. If any other value is given then the data for at least that number of miplevels must be provided.
        * @param textureUsage The requested usage for the resource.
        * @param pInitData If different than nullptr, pointer to a buffer containing data to initialize the texture with.
        * @return A pointer to a new texture, or nullptr if creation failed.
        */
        Texture* Create1DTexture(uint32_t width, PixelFormat format, uint32_t arraySize = 1, uint32_t mipLevels = 0, TextureUsage textureUsage = TextureUsage::Sampled, const void* pInitData = nullptr);

        /**
        * Create a 2D texture.
        *
        * @param width The width of the texture.
        * @param height The height of the texture.
        * @param format The format of the texture.
        * @param arraySize The array size of the texture.
        * @param mipLevels if equal to 0 then an entire mip chain will be generated from mip level 0. If any other value is given then the data for at least that number of miplevels must be provided.
        * @param textureUsage The requested usage for the texture.
        * @param pInitData If different than nullptr, pointer to a buffer containing data to initialize the texture with.
        * @return A pointer to a new texture, or nullptr if creation failed.
        */
        Texture* Create2DTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t arraySize = 1, uint32_t mipLevels = 0, TextureUsage textureUsage = TextureUsage::Sampled, const void* pInitData = nullptr);

        /**
        * Create a 2D multi-sampled texture.
        *
        * @param width The width of the texture.
        * @param height The height of the texture.
        * @param format The format of the texture.
        * @param arraySize The array size of the texture.
        * @param mipLevels if equal to 0 then an entire mip chain will be generated from mip level 0. If any other value is given then the data for at least that number of miplevels must be provided.
        * @param textureUsage The requested usage for the texture.
        * @param pInitData If different than nullptr, pointer to a buffer containing data to initialize the texture with.
        * @return A pointer to a new texture, or nullptr if creation failed
        */
        Texture* Create2DMultisampleTexture(uint32_t width, uint32_t height, PixelFormat format, SampleCount samples, uint32_t arraySize = 1, TextureUsage textureUsage = TextureUsage::Sampled, const void* pInitData = nullptr);

        /**
        * Create a new 3D texture.
        *
        * @param width The width of the texture.
        * @param height The height of the texture.
        * @param depth The depth of the texture.
        * @param format The format of the texture.
        * @param mipLevels if equal to 0 then an entire mip chain will be generated from mip level 0. If any other value is given then the data for at least that number of miplevels must be provided.
        * @param textureUsage The requested usage for the texture.
        * @param pInitData If different than nullptr, pointer to a buffer containing data to initialize the texture with.
        * @return A pointer to a new texture, or nullptr if creation failed
        */
        Texture* Create3DTexture(uint32_t width, uint32_t height, uint32_t depth, PixelFormat format, uint32_t mipLevels = 0, TextureUsage textureUsage = TextureUsage::Sampled, const void* pInitData = nullptr);

        /**
        * Create a new cubemap texture.
        *
        * @param width The width of the texture.
        * @param height The height of the texture.
        * @param format The format of the texture.
        * @param arraySize The array size of the texture.
        * @param mipLevels if equal to 0 then an entire mip chain will be generated from mip level 0. If any other value is given then the data for at least that number of miplevels must be provided.
        * @param textureUsage The requested usage for the texture.
        * @param pInitData If different than nullptr, pointer to a buffer containing data to initialize the texture with.
        * @return A pointer to a new texture, or nullptr if creation failed
        */
        Texture* CreateCubeTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t arraySize = 1, uint32_t mipLevels = 0, TextureUsage textureUsage = TextureUsage::Sampled, const void* pInitData = nullptr);

        /**
        * Create new framebuffer with given descriptor.
        *
        * @param descriptor The framebuffer descriptor.
        * @return A pointer to a new Framebuffer, or nullptr if creation failed.
        */
        Framebuffer* CreateFramebuffer(const FramebufferDescriptor* descriptor);

        /**
        * Create new buffer with given descriptor.
        *
        * @param descriptor The buffer descriptor.
        * @return A pointer to a new Buffer, or nullptr if creation failed.
        */
        Buffer* CreateBuffer(const BufferDescriptor* descriptor, const void* pInitData);

        /**
        * Create new Shader with given descriptor.
        *
        * @param descriptor The shader descriptor.
        * @return A pointer to a new Shader, or nullptr if creation failed.
        */
        Shader* CreateShader(const ShaderDescriptor* descriptor);

        /// Deferr destroy sampler handle.
        void DestroySampler(SamplerHandle handle);

#ifdef ALIMER_VULKAN
        VkInstance GetVkInstance() const { return _instance; }
        VkPhysicalDevice GetVkPhysicalDevice() const { return _physicalDevice; }
        VkDevice GetVkDevice() const { return _device; }
#endif

    protected:
        /// Add a GPUResource to keep track of. 
        void TrackResource(GPUResource* resource);

        /// Remove a GPUResource.
        void UntrackResource(GPUResource* resource);

        void OnAfterCreated();

    private:
        virtual void WaitIdleImpl() = 0;
        virtual bool InitializeImpl(const SwapChainDescriptor* descriptor) = 0;
        virtual void Finalize() {}
        virtual bool BeginFrameImpl() = 0;
        virtual void EndFrameImpl() = 0;

        CommandContext* AllocateContext(QueueType type);
        void FreeContext(CommandContext* context);
        virtual CommandContext* CreateCommandContext(QueueType type) = 0;

    protected:
        /// Constructor.
        GraphicsDevice(GraphicsBackend backend, PhysicalDevicePreference devicePreference, bool validation);

        GraphicsBackend             _backend;
        PhysicalDevicePreference    _devicePreference;
        bool                        _validation = false;
        bool                        _initialized = false;
        bool                        _inBeginFrame = false;
        uint64_t                    _frameIndex = 0;
        GraphicsDeviceFeatures      _features = {};

        std::mutex                  _contextAllocationMutex;
        std::vector<std::unique_ptr<CommandContext>> _contextPool[4];
        std::queue<CommandContext*> _availableContexts[4];
        std::vector<GPUResource*>   _gpuResources;
        std::mutex                  _gpuResourceMutex;
        Sampler*                    _pointSampler = nullptr;
        Sampler*                    _linearSampler = nullptr;

        // Backend types
#ifdef ALIMER_VULKAN
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
#endif
    };

    ALIMER_API extern GraphicsDevice* graphics;
}
