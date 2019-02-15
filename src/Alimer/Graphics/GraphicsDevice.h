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

    /// Low-level graphics module.
    class ALIMER_API GraphicsDevice : public Object
    {
        friend class GPUResource;
        friend class CommandContext;

        ALIMER_OBJECT(GraphicsDevice, Object);

    public:
        /// Constructor.
        static GraphicsDevice* Create(GraphicsBackend preferredBackend, PhysicalDevicePreference devicePreference = PhysicalDevicePreference::Discrete, bool validation = false);

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
        Framebuffer* GetDefaultFramebuffer() const;

        /// Return whether rendering initialized.
        bool IsInitialized() const { return _initialized; }

        /// Get the backend.
        inline GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device features.
        inline const GraphicsDeviceFeatures& GetFeatures() const { return _features; }

        /// Get the device limits
        inline const GraphicsDeviceLimits& GetLimits() const { return _limits; }

        /// Get the default command context.
        inline CommandContext& GetContext() { return *_context.Get(); }

    protected:
        /// Add a GPUResource to keep track of. 
        void TrackResource(GPUResource* resource);

        /// Remove a GPUResource.
        void UntrackResource(GPUResource* resource);

        void OnAfterCreated();

    private:
        virtual void Finalize() {}
        virtual bool InitializeImpl(const char* applicationName, const SwapChainDescriptor* descriptor) = 0;
        virtual void WaitIdleImpl() = 0;
        virtual bool BeginFrameImpl() = 0;
        virtual void EndFrameImpl() = 0;

        CommandContext* AllocateContext(QueueType type);
        void FreeContext(CommandContext* context);
        //virtual CommandContext* CreateCommandContext(QueueType type) = 0;

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
        GraphicsDeviceLimits              _limits = {};
        SharedPtr<CommandContext>   _context;
        std::mutex                  _contextAllocationMutex;
        std::vector<std::unique_ptr<CommandContext>> _contextPool[4];
        std::queue<CommandContext*> _availableContexts[4];
        std::vector<GPUResource*>   _gpuResources;
        std::mutex                  _gpuResourceMutex;
        Sampler*                    _pointSampler = nullptr;
        Sampler*                    _linearSampler = nullptr;
    };

    ALIMER_API extern GraphicsDevice* graphics;
}
