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
#include "../Graphics/GraphicsDeviceFeatures.h"
#include "../Graphics/CommandContext.h"
#include "../Graphics/Buffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Sampler.h"
#include "../Graphics/Pipeline.h"
#include "../Graphics/RenderWindow.h"
#include <set>
#include <mutex>

namespace Alimer
{
    /// Low-level 3D graphics module.
    class ALIMER_API GPUDevice : public Object
    {
        ALIMER_OBJECT(GPUDevice, Object);

    protected:
        /// Constructor.
        GPUDevice(GraphicsBackend backend, bool validation);

    public:
        /// Destructor.
        ~GPUDevice() override;

        static GPUDevice* Create(GraphicsBackend preferredBackend, bool validation);

        /// Register object.
        static void RegisterObject();

        /// Shutdown the Graphics module.
        static void Shutdown();

        /// Check if backend is supported
        static bool IsBackendSupported(GraphicsBackend backend);

        /// Get all available backends.
        static std::set<GraphicsBackend> GetAvailableBackends();

        /// Get default best supported platform backend.
        static GraphicsBackend GetDefaultPlatformBackend();

        /// Initialize graphics with given settings.
        virtual bool Initialize(const RenderWindowDescriptor* mainWindowDescriptor);

        /// Wait for a device to become idle.
        virtual bool WaitIdle() = 0;

        /// Finishes the current frame and advances to next one.
        uint64_t Frame();

        /// Add a GPUResource to keep track of. 
        void TrackResource(GPUResource* resource);

        /// Remove a GPUResource.
        void UntrackResource(GPUResource* resource);

        /// Get whether grapics has been initialized.
        bool IsInitialized() const { 
            return _initialized; 
        }

        /// Get the backend.
        GraphicsBackend GetBackend() const {
            return _backend; 
        }

        /// Get the device features.
        const GraphicsDeviceFeatures& GetFeatures() const {
            return _features;
        }

        /// Get the main RenderWindow.
        RenderWindow* GetMainWindow() const {
            return _mainWindow;
        }

        /// Get the immediate command context.
        CommandContext& GetImmediateContext() const {
            return *_immediateCommandContext;
        }

        /// Create new CommandContext
        CommandContext& Begin(const String& name = "");

        void NotifyValidationError(const char* message);

        Buffer* CreateBuffer(BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData = nullptr, const std::string& name = "");

        Texture* CreateTexture1D(uint32_t width, uint32_t mipLevels, uint32_t arrayLayers, PixelFormat format, TextureUsage usage, const void* initialData = nullptr);
        Texture* CreateTexture2D(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLayers, PixelFormat format, TextureUsage usage, SampleCount samples = SampleCount::Count1, const void* initialData = nullptr);
        Texture* CreateTextureCube(uint32_t size, uint32_t mipLevels, uint32_t arrayLayers, PixelFormat format, TextureUsage usage, const void* initialData = nullptr);
        Texture* CreateTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, PixelFormat format, TextureUsage usage, const void* initialData = nullptr);

        Sampler* CreateSampler(const SamplerDescriptor* descriptor);
        Framebuffer* CreateFramebuffer(const PODVector<FramebufferAttachment>& colorAttachments, const FramebufferAttachment* depthStencilAttachment = nullptr);
        Framebuffer* CreateFramebuffer(uint32_t colorAttachmentsCount, const FramebufferAttachment* colorAttachments, const FramebufferAttachment* depthStencilAttachment = nullptr);

    private:
        virtual void OnFrame() {};
        virtual Buffer* CreateBufferCore(BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData, const std::string& name) = 0;
        virtual Texture* CreateTextureCore(TextureType type, uint32_t width, uint32_t height,
            uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, PixelFormat format, 
            TextureUsage usage, SampleCount samples, const void* initialData) = 0;
        virtual Sampler* CreateSamplerCore(const SamplerDescriptor* descriptor) = 0;
        virtual Framebuffer* CreateFramebufferCore(uint32_t colorAttachmentsCount, const FramebufferAttachment* colorAttachments, const FramebufferAttachment* depthStencilAttachment) = 0;

    protected:
        virtual void Finalize();

        GraphicsBackend         _backend;
        bool                    _validation;
        bool                    _initialized;
        uint64_t                _frameIndex;
        GraphicsDeviceFeatures  _features;
        PODVector<GPUResource*> _gpuResources;
        std::mutex              _gpuResourceMutex;
        RenderWindow*           _mainWindow = nullptr;
        CommandContext*         _immediateCommandContext = nullptr;
    };
}
