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
#include "../Graphics/Window.h"
#include "../Graphics/CommandBuffer.h"
#include "../Graphics/GraphicsDeviceFeatures.h"
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

    struct GraphicsDeviceDescriptor
    {
        GraphicsBackend preferredBackend = GraphicsBackend::Count;
        GpuPreference devicePreference = GpuPreference::HighPerformance;
        bool validation = false;
    };

    /// Low-level graphics module.
    class ALIMER_API GraphicsDevice 
    {
        friend class GPUResource;
    public:
        /// Create factory.
        static GraphicsDevice* Create(const char* applicationName, const GraphicsDeviceDescriptor* descriptor);

        /// Destructor.
        virtual ~GraphicsDevice();

        /// Initialize device with given swap chain descriptor.
        bool Initialize(const SwapChainDescriptor* descriptor);

        /// Begin rendering frame and returns command buffer for recording.
        virtual bool BeginFrame();

        /// End frame rendering and swap buffers.
        void EndFrame();

        /// Get the backend.
        inline GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device features.
        inline const GraphicsDeviceCapabilities& GetCaps() const { return _caps; }

        /// Return whether rendering initialized.
        bool IsInitialized() const { return _initialized; }

        /// Get the main rendering window.
        Window* GetRenderWindow() const { return _renderWindow.get(); }

        /// Get the main rendering context.
        CommandBuffer* GetRenderContext() const { return _renderContext.Get(); }

        /// Get the current backbuffer texture.
        virtual Texture* GetCurrentTexture() const = 0;

        /// Get the backbuffer depth-stencil texture.
        virtual Texture* GetDepthStencilTexture() const = 0;

        /// Get the backbuffer multisample color texture.
        virtual Texture* GetMultisampleColorTexture() const = 0;

    protected:
        /// Add a GPUResource to keep track of. 
        void TrackResource(GPUResource* resource);

        /// Remove a GPUResource.
        void UntrackResource(GPUResource* resource);

        void Finalize();

    private:
        // Backend methods
        virtual bool InitializeImpl(const SwapChainDescriptor* descriptor) = 0;
        virtual void Tick() = 0;

    protected:
        /// Constructor.
        GraphicsDevice(GraphicsBackend backend, const GraphicsDeviceDescriptor* descriptor);

        /// Implementation.
        GraphicsBackend             _backend;
        GpuPreference               _devicePreference;
        bool                        _validation = false;
        bool                        _initialized = false;
        GraphicsDeviceInfo          _info = {};
        GraphicsDeviceCapabilities  _caps = {};
        std::vector<GPUResource*>   _gpuResources;
        std::mutex                  _gpuResourceMutex;
        Sampler*                    _pointSampler = nullptr;
        Sampler*                    _linearSampler = nullptr;
        SharedPtr<CommandBuffer>    _renderContext;
        std::unique_ptr<Window>     _renderWindow;
    };

    ALIMER_API extern GraphicsDevice* graphics;
}
