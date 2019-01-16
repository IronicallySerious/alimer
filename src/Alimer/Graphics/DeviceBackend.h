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

#include "Graphics/Types.h"
#include "Graphics/VertexFormat.h"
#include "Graphics/GraphicsDeviceFeatures.h"

namespace alimer
{
    class GPUTexture
    {
    public:
        virtual ~GPUTexture() = default;
    };

    class GPUFramebuffer
    {
    public:
        virtual ~GPUFramebuffer() = default;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetLayers() const = 0;

        virtual void SetColorAttachment(uint32_t index, GPUTexture* colorTexture, uint32_t level, uint32_t slice) = 0;
        virtual void SetDepthStencilAttachment(GPUTexture* depthStencilTexture, uint32_t level, uint32_t slice) = 0;
    };

    class GPUSampler
    {
    public:
        virtual ~GPUSampler() = default;
    };

    class GPUBuffer
    {
    public:
        virtual ~GPUBuffer() = default;
    };

    class GPUShader
    {
    public:
        virtual ~GPUShader() = default;
    };

    class GPUSwapChain
    {
    public:
        virtual ~GPUSwapChain() = default;

        virtual uint32_t GetTextureCount() const = 0;
        virtual uint32_t GetCurrentBackBuffer() const = 0;
        virtual GPUTexture* GetBackBufferTexture(uint32_t index) const = 0;

        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual void Present() = 0;
    };

    class GPUCommandBuffer
    {
    public:
        virtual ~GPUCommandBuffer() = default;

        virtual uint64_t Flush(bool waitForCompletion) = 0;
        virtual void PushDebugGroup(const char* name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(const char* name) = 0;

    };

    class DeviceBackend
    {
    protected:
        DeviceBackend(GraphicsBackend backend, bool validation);

    public:
        virtual ~DeviceBackend();

        virtual bool WaitIdle() = 0;
        virtual void Tick() = 0;

        virtual GPUCommandBuffer* GetDefaultCommandBuffer() const { return _defaultCommandBuffer; }

        virtual GPUSwapChain* CreateSwapChain(const SwapChainDescriptor* descriptor) = 0;
        virtual GPUTexture* CreateTexture(const TextureDescriptor* descriptor, void* nativeTexture, const void* initialData) = 0;
        virtual GPUSampler* CreateSampler(const SamplerDescriptor* descriptor) = 0;

        GraphicsBackend GetBackend() const { return _backend; }
        const GPULimits& GetLimits() const { return _limits; }
        const GraphicsDeviceFeatures& GetFeatures() const { return _features; }

    protected:
        GraphicsBackend         _backend;
        bool                    _validation;
        GPULimits               _limits{};
        GraphicsDeviceFeatures  _features{};
        GPUCommandBuffer*       _defaultCommandBuffer = nullptr;
    };
}
