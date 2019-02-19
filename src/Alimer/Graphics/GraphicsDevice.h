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
#include "../Graphics/GPUBackend.h"
#include "../Graphics/GraphicsDeviceFeatures.h"
#include "../Graphics/CommandContext.h"
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
        GpuPreference devicePreference = GpuPreference::HighPerformance;
        bool validation = false;
        bool headless = false;
    };

    class GraphicsImpl;

    /// Low-level graphics module.
    class ALIMER_API GraphicsDevice : public Object
    {
        friend class GPUResource;
        friend class CommandContext;

        ALIMER_OBJECT(GraphicsDevice, Object);

    public:
        /// Is backend supported?
        static bool IsSupported();

        /// Create factory.
        static GraphicsDevice* Create(const char* applicationName, const GraphicsDeviceDescriptor* descriptor);

        /// Destructor.
        virtual ~GraphicsDevice() override;

        /// Initialize device with given swap chain descriptor.
        bool Initialize(const SwapChainDescriptor* descriptor);

        /// Begin rendering frame and returns command buffer for recording.
        bool BeginFrame();

        /// End frame.
        uint32_t EndFrame();

        /// Get the backend.
        inline GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device features.
        const GraphicsDeviceFeatures& GetFeatures() const;

        /// Get the device limits
        const GraphicsDeviceLimits& GetLimits() const;

        /// Get the frame command context.
        SharedPtr<CommandContext> GetContext() const;

        /// Return whether rendering initialized.
        bool IsInitialized() const { return _initialized; }

        /// Return graphics implementation, which holds the actual API-specific resources.
        GraphicsImpl* GetImpl() const { return _impl; }

        /// Get the current backbuffer texture;
        //virtual SharedPtr<Texture> GetCurrentColorTexture() const = 0;
        /// Get the current backbuffer texture;
        //virtual SharedPtr<Texture> GetCurrentDepthStencilTexture() const = 0;
        /// Get the current backbuffer texture;
        //virtual SharedPtr<Texture> GetCurrentMultisampleColorTexture() const = 0;

    protected:
        /// Add a GPUResource to keep track of. 
        void TrackResource(GPUResource* resource);

        /// Remove a GPUResource.
        void UntrackResource(GPUResource* resource);

        void OnAfterCreated();

    private:
        /// Constructor.
        GraphicsDevice(const char* applicationName, const GraphicsDeviceDescriptor* descriptor);

        /// Implementation.
        GraphicsImpl*               _impl = nullptr;
        GraphicsBackend             _backend;
        GpuPreference               _devicePreference;
        bool                        _headless = false;
        bool                        _validation = false;
        bool                        _initialized = false;
        std::vector<GPUResource*>   _gpuResources;
        std::mutex                  _gpuResourceMutex;
        Sampler*                    _pointSampler = nullptr;
        Sampler*                    _linearSampler = nullptr;
        uint32_t                    _frameId = 0;
    };

    ALIMER_API extern GraphicsDevice* graphics;
}
