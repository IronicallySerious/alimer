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
        GraphicsBackend preferredBackend = GraphicsBackend::Default;
        PhysicalDevicePreference devicePreference = PhysicalDevicePreference::Discrete;
        bool validation = false;
    };

    /// Low-level graphics module.
    class ALIMER_API GraphicsDevice : public Object
    {
        friend class GPUResource;
        friend class CommandContext;

        ALIMER_OBJECT(GraphicsDevice, Object);

    public:
        /// Create factory.
        static GraphicsDevice* Create(const char* applicationName, const GraphicsDeviceDescriptor* descriptor);

        static GraphicsBackend GetDefaultPlatformBackend();
        static bool IsBackendSupported(GraphicsBackend backend);

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
        inline const GraphicsDeviceFeatures& GetFeatures() const { return _features; }

        /// Get the device limits
        inline const GraphicsDeviceLimits& GetLimits() const { return _limits; }

        /// Get the frame command context.
        virtual SharedPtr<CommandContext> GetContext() const = 0;

        /// Return whether rendering initialized.
        bool IsInitialized() const { return _initialized; }

    protected:
        /// Add a GPUResource to keep track of. 
        void TrackResource(GPUResource* resource);

        /// Remove a GPUResource.
        void UntrackResource(GPUResource* resource);

        void OnAfterCreated();

    private:
        virtual void Finalize() {}
        virtual bool BeginFrameImpl() = 0;
        virtual void EndFrame(uint32_t frameId) = 0;
        virtual bool InitializeImpl(const SwapChainDescriptor* descriptor) = 0;

    protected:
        /// Constructor.
        GraphicsDevice(GraphicsBackend backend, PhysicalDevicePreference devicePreference, bool validation);

        GraphicsBackend             _backend;
        PhysicalDevicePreference    _devicePreference;
        bool                        _validation = false;
        GraphicsDeviceFeatures      _features = {};
        GraphicsDeviceLimits        _limits = {};
        bool                        _initialized = false;
        std::vector<GPUResource*>   _gpuResources;
        std::mutex                  _gpuResourceMutex;
        Sampler*                    _pointSampler = nullptr;
        Sampler*                    _linearSampler = nullptr;
        uint32_t                    _frameId = 0;
    };

    ALIMER_API extern GraphicsDevice* graphics;
}
