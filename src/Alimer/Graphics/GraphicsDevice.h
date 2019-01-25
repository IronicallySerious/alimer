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
#include "../Graphics/CommandContext.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Pipeline.h"
#include <mutex>

namespace alimer
{
    class SwapChain;
    class Shader;
    class Buffer;
    class Sampler;

    struct GraphicsDeviceDescriptor 
    {
        SwapChainDescriptor     swapchain = {};
    };

    class PhysicalDevice;

    /// Low-level graphics module.
    class ALIMER_API GraphicsDevice : public Object
    {
        ALIMER_OBJECT(GraphicsDevice, Object);

    public:
        /// Check if device is supported on running platform.
        static bool IsSupported();

        /// Constructor.
        GraphicsDevice(bool validation, bool headless);

        /// Destructor.
        ~GraphicsDevice() override;

        bool Initialize(const GraphicsDeviceDescriptor* descriptor);

        /// Add a GPUResource to keep track of. 
        void TrackResource(GPUResource* resource);

        /// Remove a GPUResource.
        void UntrackResource(GPUResource* resource);

        /// Get the backend.
        GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device limits.
        const GPULimits& GetLimits() const { return _limits; }

        /// Get the device features.
        const GraphicsDeviceFeatures& GetFeatures() const { return _features; }

        /// Get the selected physical device.
        PhysicalDevice* GetPhysicalDevice() const { return _physicalDevice; }

        /** 
        * Get the default render-context.
        * The default render-context is managed completely by the device. 
        * The user should just queue commands into it, the device will take care of allocation, submission and synchronization
        */
        CommandContext& GetContext() const { return *_renderContext; }

        /**
        * Get the current backbuffer framebuffer.
        */
        //virtual Framebuffer* GetBackbufferFramebuffer() const = 0;

        /// Begin rendering frame.
        bool BeginFrame();

        /// Finishes the current frame and advances to next one.
        uint64_t EndFrame();

        /// Wait for a device to become idle.
        bool WaitIdle();

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
        * Create new Sampler with given descriptor.
        *
        * @param descriptor The sampler descriptor.
        * @return A pointer to a new Sampler, or nullptr if creation failed.
        */
        Sampler* CreateSampler(const SamplerDescriptor* descriptor);

        /**
        * Create new Shader with given descriptor.
        *
        * @param descriptor The shader descriptor.
        * @return A pointer to a new Shader, or nullptr if creation failed.
        */
        Shader* CreateShader(const ShaderDescriptor* descriptor);

        void NotifyValidationError(const char* message);

#if defined(ALIMER_VULKAN)
        VkInstance GetVkInstance() const { 
            return _instance; 
        }

        VkDevice GetVkDevice() const {
            return _device; 
        }

        uint32_t GetVkGraphicsQueueFamily() const {
            return _graphicsQueueFamily;
        }

        uint32_t GetVkComputeQueueFamily() const {
            return _computeQueueFamily;
        }

        uint32_t GetVkTransferQueueFamily() const {
            return _transferQueueFamily;
        }
#endif /* ALIMER_VULKAN */

    protected:
        void OnAfterCreated();

    private:
        void PlatformConstruct();
        void PlatformDestroy();
        bool PlatformInitialize(const GraphicsDeviceDescriptor* descriptor);
        //virtual bool BeginFrameImpl() = 0;
        //virtual void EndFrameImpl() = 0;

    private:
        /// Implementation.
        bool                            _validation;
        bool                            _headless;
        bool                            _initialized;
        GraphicsBackend                 _backend = GraphicsBackend::Invalid;
        bool                            _inBeginFrame = false;
        uint64_t                        _frameIndex = 0;
        Vector<PhysicalDevice*>         _physicalDevices;
        PhysicalDevice*                 _physicalDevice;

        GPULimits                       _limits = {};
        GraphicsDeviceFeatures          _features = {};
        PODVector<GPUResource*>         _gpuResources;
        std::mutex                      _gpuResourceMutex;
        SwapChain*                      _mainSwapChain = nullptr;
        SharedPtr<CommandContext>       _renderContext;
        Sampler*                        _pointSampler = nullptr;
        Sampler*                        _linearSampler = nullptr;

#if defined(ALIMER_D3D11)
#elif defined(ALIMER_D3D12)
#elif defined(ALIMER_VULKAN)
        VkInstance                      _instance = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT        _debugCallback = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT        _debugMessenger = VK_NULL_HANDLE;
        uint32_t                        _graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t                        _computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t                        _transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        VkDevice                        _device = VK_NULL_HANDLE;
        VkQueue                         _graphicsQueue = VK_NULL_HANDLE;
        VkQueue                         _computeQueue = VK_NULL_HANDLE;
        VkQueue                         _transferQueue = VK_NULL_HANDLE;
        /* Features */
        bool                            _supportsDedicated = false;
        bool                            _supportsImageFormatList = false;
        bool                            _supportsGoogleDisplayTiming = false;
        bool                            _supportsDebugMarker = false;
        bool                            _supportsDebugUtils = false;
        bool                            _supportsMirrorClampToEdge = false;
#elif defined(ALIMER_OPENGL)
#endif
    };
}
