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
#include "../Graphics/Types.h"
#include "../Graphics/Window.h"
#include "../Graphics/CommandBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Pipeline.h"
#include <memory>
#include <vector>
#include <queue>
#include <mutex>

namespace alimer
{
    class Sampler;
    class GraphicsImpl;

    /// Low-level graphics module.
    class ALIMER_API GraphicsDevice final : public Object
    {
        friend class GPUResource;
        ALIMER_OBJECT(GraphicsDevice, Object);

    private:
        /// Constructor.
        GraphicsDevice(const char* applicationName, GpuPreference devicePreference);

    public:
        /// Register object factory.
        static void RegisterObject();

        /// Create factory.
        static GraphicsDevice* Create(const char* applicationName, GpuPreference devicePreference = GpuPreference::Default);

        /// Destructor.
        ~GraphicsDevice();

        /// Set graphics mode. Create the window and rendering context if not created yet. Return true on success.
        bool SetMode(const uvec2& size, bool resizable = true, bool fullscreen = false, SampleCount samples = SampleCount::Count1);

        /// Begin rendering frame and returns command buffer for recording.
        bool BeginFrame();

        /// End frame rendering and swap buffers.
        void EndFrame();

        /// Get the backend.
        GraphicsBackend GetBackend() const;

        /// Get the device info.
        const GraphicsDeviceInfo& GetInfo() const;

        /// Get the device features.
        const GraphicsDeviceCapabilities& GetCaps() const;

        /// Return whether rendering initialized.
        bool IsInitialized() const { return _initialized; }

        /// Get the main rendering window.
        Window* GetRenderWindow() const { return _renderWindow.Get(); }

        /// Get the main rendering context.
        CommandBuffer* GetRenderContext() const { return _renderContext.Get(); }

        /// Get the current backbuffer texture.
        //virtual Texture* GetCurrentTexture() const = 0;

        /// Get the backbuffer depth-stencil texture.
        //virtual Texture* GetDepthStencilTexture() const = 0;

        /// Get the backbuffer multisample color texture.
        //virtual Texture* GetMultisampleColorTexture() const = 0;

         /// Return graphics implementation, which holds the actual API-specific resources.
        GraphicsImpl* GetImpl() const { return _impl; }

    protected:
        /// Add a GPUResource to keep track of. 
        void TrackResource(GPUResource* resource);

        /// Remove a GPUResource.
        void UntrackResource(GPUResource* resource);

        void Finalize();

    private:
        // Backend methods
        //virtual bool InitializeImpl(const SwapChainDescriptor* descriptor) = 0;
        //virtual void Tick() = 0;

    private:
        /// Implementation.
        GraphicsImpl* _impl;

    protected:
        /// Implementation.
        bool                        _initialized = false;
        std::vector<GPUResource*>   _gpuResources;
        std::mutex                  _gpuResourceMutex;
        Sampler*                    _pointSampler = nullptr;
        Sampler*                    _linearSampler = nullptr;
        SharedPtr<CommandBuffer>    _renderContext;
        UniquePtr<Window>     _renderWindow;
    };
}
