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
#include "../Graphics/Buffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Pipeline.h"
#include <set>
#include <mutex>

namespace alimer
{
    struct GPUDeviceImpl;
    class SwapChain;
    class Sampler;

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

        /// Wait for a device to become idle.
        bool WaitIdle();

        /// Finishes the current frame and advances to next one.
        uint64_t Frame();

        /// Add a GPUResource to keep track of. 
        void TrackResource(GPUResource* resource);

        /// Remove a GPUResource.
        void UntrackResource(GPUResource* resource);

        /// Get the backend.
        GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device limits.
        const GPULimits& GetLimits() const;

        /// Get the device features.
        const GraphicsDeviceFeatures& GetFeatures() const;

        /// Return graphics implementation, which holds the actual API-specific resources.
        GPUDeviceImpl* GetImpl() const { return _impl; }

        /// Get the immediate command context.
        CommandContext& GetImmediateContext() const { return *_immediateCommandContext; }

        /// Create new CommandContext
        CommandContext& Begin(const String& name = "");

        void NotifyValidationError(const char* message);

        SwapChain* CreateSwapChain(const SwapChainDescriptor* descriptor);

        Texture* CreateTexture1D(uint32_t width, uint32_t mipLevels, uint32_t arrayLayers, PixelFormat format, TextureUsage usage, const void* initialData = nullptr);
        Texture* CreateTextureCube(uint32_t size, uint32_t mipLevels, uint32_t arrayLayers, PixelFormat format, TextureUsage usage, const void* initialData = nullptr);
        Texture* CreateTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, PixelFormat format, TextureUsage usage, const void* initialData = nullptr);

        Sampler* CreateSampler(const SamplerDescriptor* descriptor);

    private:
        virtual void OnFrame() {};
        virtual SwapChain* CreateSwapChainImpl(const SwapChainDescriptor* descriptor) = 0;
        virtual Sampler* CreateSamplerImpl(const SamplerDescriptor* descriptor) = 0;

        GPUDeviceImpl* _impl;

    protected:
        virtual void Finalize();

        GraphicsBackend         _backend;
        bool                    _validation;
        uint64_t                _frameIndex;
        PODVector<GPUResource*> _gpuResources;
        std::mutex              _gpuResourceMutex;
        CommandContext*         _immediateCommandContext = nullptr;
    };
}
