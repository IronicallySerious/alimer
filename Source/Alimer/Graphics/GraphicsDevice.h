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

#include "../Base/Cache.h"
#include "../Core/Object.h"
#include "../Graphics/GraphicsDeviceFeatures.h"
#include "../Graphics/CommandContext.h"
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Pipeline.h"
#include "../Graphics/ShaderCompiler.h"
#include <vector>
#include <mutex>
#include <set>

namespace Alimer
{
    struct SwapchainDescriptor
    {
        uint32_t width;
        uint32_t height;
        /// Preferred color format.
        PixelFormat colorFormat = PixelFormat::BGRA8UNorm;
        /// Preferred depth stencil format.
        PixelFormat depthStencilFormat = PixelFormat::D24UNormS8;
        /// Preferred samples.
        SampleCount samples = SampleCount::Count1;
        /// Per platform window handle.
        void* windowHandle = nullptr;
    };

    struct RenderingSettings
    {
        SwapchainDescriptor swapchain;
    };

    /// Low-level 3D graphics module.
    class ALIMER_API Graphics : public Object
    {
        ALIMER_OBJECT(Graphics, Object);

    protected:
        /// Constructor.
        Graphics(GraphicsBackend backend, bool validation);

    public:
        /// Register object.
        static void RegisterObject();

        /// Destructor.
        virtual ~Graphics() override;

        /// Check if backend is supported
        static bool IsBackendSupported(GraphicsBackend backend);

        /// Get all available backends.
        static std::set<GraphicsBackend> GetAvailableBackends();

        /// Create new graphics device instance.
        static Graphics* Create(GraphicsBackend prefferedBackend = GraphicsBackend::Default, bool validation = false);

        /// Initialize graphics with given settings.
        virtual bool Initialize(const RenderingSettings& settings);

        /// Wait for a device to become idle.
        virtual bool WaitIdle() = 0;

        /// Finishes the current frame and schedules it for display.
        uint32_t Present();

        /// Add a GraphicsResource to keep track of. 
        void AddGraphicsResource(GraphicsResource* resource);

        /// Remove a GraphicsResource.
        void RemoveGraphicsResource(GraphicsResource* resource);

        /// Create new buffer with given descriptor and optional initial data.
        GpuBuffer* CreateBuffer(const BufferDescriptor* descriptor, const void* initialData = nullptr);

        /// Create new buffer with given descriptor and optional initial data.
        Texture* CreateTexture(const TextureDescriptor* descriptor, const ImageLevel* initialData = nullptr);

        /// Create new framebuffer with given descriptor.
        Framebuffer* CreateFramebuffer(const FramebufferDescriptor* descriptor);

        /// Get whether grapics has been initialized.
        bool IsInitialized() const { return _initialized; }

        /// Get the backend.
        GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device features.
        const GraphicsDeviceFeatures& GetFeatures() const { return _features; }

        /// Get the main Swapchain current framebuffer.
        virtual Framebuffer* GetSwapchainFramebuffer() const = 0;

        /// Get the default command context.
        const SharedPtr<CommandContext>& GetContext() const { return _context; }

        void NotifyValidationError(const char* message);

    protected:
        virtual void Shutdown();
        virtual void PresentImpl() = 0;
        virtual GpuBuffer* CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData) = 0;
        virtual Texture* CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData) = 0;
        virtual Framebuffer* CreateFramebufferImpl(const FramebufferDescriptor* descriptor) = 0;


        GraphicsBackend _backend = GraphicsBackend::Empty;
        bool _validation;
        bool _initialized = false;
        RenderingSettings _settings = {};
        GraphicsDeviceFeatures _features = {};
        std::vector<GraphicsResource*> _gpuResources;
        std::mutex _gpuResourceMutex;
        SharedPtr<CommandContext> _context;

    private:
        uint32_t _frameIndex = 0;
    };

    /// Register Graphics related object factories and attributes.
    ALIMER_API void RegisterGraphicsLibrary();
}
