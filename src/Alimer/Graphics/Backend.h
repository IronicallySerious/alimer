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

#include "../Graphics/Types.h"
#include "../Graphics/VertexFormat.h"
#include "../Graphics/PixelFormat.h"
#include "../Graphics/GraphicsDeviceFeatures.h"
#include "../Core/Log.h"

namespace alimer
{
    class GPUTexture
    {
    public:
        virtual ~GPUTexture() = default;
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

    class GPUSwapChain
    {
    public:
        virtual ~GPUSwapChain() = default;

        virtual uint32_t GetTextureCount() const = 0;
        virtual uint32_t GetCurrentBackBuffer() const = 0;
        virtual GPUTexture* GetBackBufferTexture(uint32_t index) const = 0;
    };

    class GPUCommandBuffer
    {
    public:
        virtual ~GPUCommandBuffer() = default;

        virtual void PushDebugGroup(const char* name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(const char* name) = 0;
    };

    class GPUDevice
    {
    protected:
        GPUDevice(GraphicsBackend backend, bool validation, bool headless)
            : _backend(backend)
            , _validation(validation)
            , _headless(headless)
        {
        }

    public:
        virtual ~GPUDevice() = default;

        virtual void WaitIdle() = 0;

        GraphicsBackend GetBackend() const { return _backend; }
        const GraphicsDeviceFeatures& GetFeatures() const { return _features; }
        virtual GPUCommandBuffer* GetDefaultCommandBuffer() const { return _defaultCommandBuffer; }

        virtual bool BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual GPUTexture* CreateTexture(const TextureDescriptor* descriptor, void* nativeTexture, const void* pInitData) = 0;
        virtual GPUSampler* CreateSampler(const SamplerDescriptor* descriptor) = 0;
        virtual GPUBuffer* CreateBuffer(const BufferDescriptor* descriptor, const void* pInitData) = 0;

    protected:
        GraphicsBackend _backend;
        bool _validation;
        bool _headless;
        GraphicsDeviceFeatures _features = {};
        GPUCommandBuffer* _defaultCommandBuffer = nullptr;
    };
};
