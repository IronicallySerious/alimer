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
    class Shader;
    class Buffer;
    class Sampler;
    class RenderWindow;
    struct WindowResizeEvent;

    /// Low-level graphics module.
    class ALIMER_API GraphicsDevice : public Object
    {
        ALIMER_OBJECT(GraphicsDevice, Object);

    public:
        /// Get default best supported platform backend.
        static GraphicsBackend GetDefaultPlatformBackend();

        /// Check if backend is supported.
        static bool IsBackendSupported(GraphicsBackend backend);

        /// Constructor.
        static GraphicsDevice* Create(GraphicsBackend preferredBackend = GraphicsBackend::Default, bool validation = false, bool headless = false);

        /// Destructor.
        ~GraphicsDevice() override;

        /// Set graphics mode. Create the OS window and rendering context if not created yet. Return true on success.
        bool SetMode(const String& title, const IntVector2& size, bool fullscreen = false, bool resizable = false, bool vSync = true, SampleCount samples = SampleCount::Count1);

        /// Add a GPUResource to keep track of. 
        void TrackResource(GPUResource* resource);

        /// Remove a GPUResource.
        void UntrackResource(GPUResource* resource);

        /// Get the backend.
        GraphicsBackend GetBackend() const;

        /// Get the device features.
        const GraphicsDeviceFeatures& GetFeatures() const;

        /// Return the main renderin window.
        RenderWindow* GetRenderWindow() const;

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

        /// Wait device to finish all pending operations.
        virtual void WaitIdle();

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

    private:
        void OnAfterCreated();

    protected:
        /// Constructor.
        GraphicsDevice(GraphicsBackend backend, bool validation, bool headless);

    private:
        //virtual bool BeginFrameImpl() = 0;
        //virtual void EndFrameImpl() = 0;

        GraphicsBackend _backend;
        bool _inBeginFrame = false;
        uint64_t _frameIndex = 0;

    protected:
        bool _validation;
        bool _headless;
        GraphicsDeviceFeatures _features = {};

        /// OS window.
        SharedPtr<RenderWindow>         _renderWindow;

        PODVector<GPUResource*>         _gpuResources;
        std::mutex                      _gpuResourceMutex;
        SharedPtr<CommandContext>       _renderContext;
        Sampler*                        _pointSampler = nullptr;
        Sampler*                        _linearSampler = nullptr;
    };
}
