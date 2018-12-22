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

#include "../Graphics/Types.h"
#include "../Graphics/GraphicsDeviceFeatures.h"

namespace Alimer
{
    struct GPUTexture
    {
        virtual ~GPUTexture() = 0;

        virtual TextureDescriptor GetDescriptor() = 0;
    };

    struct GPUBuffer
    {
        virtual ~GPUBuffer() = 0;
    };

    struct GPUSwapChain
    {
        virtual ~GPUSwapChain() = 0;

        virtual uint32_t GetBackBufferCount() const = 0;
        virtual uint32_t GetCurrentBackBuffer() const = 0;
        virtual GPUTexture* GetBackBufferTexture(uint32_t index) const = 0;

        virtual void Configure(uint32_t width, uint32_t height) = 0;
        virtual void Present() = 0;
    };

    struct GPUFramebuffer
    {
        virtual ~GPUFramebuffer() = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetLayers() const = 0;

        virtual void SetColorAttachment(uint32_t index, GPUTexture* colorTexture, uint32_t level, uint32_t slice) = 0;
        virtual void SetDepthStencilAttachment(GPUTexture* depthStencilTexture, uint32_t level, uint32_t slice) = 0;
    };

    struct GPUShader
    {
        virtual ~GPUShader() = 0;
    };

    struct GPUSampler
    {
        virtual ~GPUSampler() = 0;
    };

    struct GPUCommandBuffer
    {
        virtual ~GPUCommandBuffer() = 0;

        virtual uint64_t Flush(bool waitForCompletion) = 0;
        
        virtual void BeginRenderPass(GPUFramebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor) = 0;
        virtual void EndRenderPass() = 0;
    };

    struct GPUDeviceImpl
    {
        virtual ~GPUDeviceImpl() = 0;

        virtual GraphicsBackend GetBackend() const = 0;

        virtual const GPULimits& GetLimits() const = 0;
        virtual const GraphicsDeviceFeatures& GetFeatures() const = 0;
        virtual GPUCommandBuffer* GetDefaultCommandBuffer() const = 0;

        virtual bool WaitIdle() = 0;
        virtual GPUSwapChain* CreateSwapChain(void* window, uint32_t width, uint32_t height, PixelFormat depthStencilFormat, bool srgb) = 0;
        virtual GPUTexture* CreateTexture(const TextureDescriptor& descriptor, const void* initialData) = 0;
        virtual GPUFramebuffer* CreateFramebuffer() = 0;
        virtual GPUBuffer* CreateBuffer(const BufferDescriptor& descriptor, const void* initialData) = 0;
        virtual GPUSampler* CreateSampler(const SamplerDescriptor& descriptor) = 0;
        virtual GPUShader* CreateShader(const char* source) = 0;
    };

    inline GPUTexture::~GPUTexture()
    {
    }

    inline GPUBuffer::~GPUBuffer()
    {
    }

    inline GPUSwapChain::~GPUSwapChain()
    {
    }

    inline GPUFramebuffer::~GPUFramebuffer()
    {
    }

    inline GPUShader::~GPUShader()
    {
    }

    inline GPUSampler::~GPUSampler()
    {
    }

    inline GPUCommandBuffer::~GPUCommandBuffer()
    {
    }

    inline GPUDeviceImpl::~GPUDeviceImpl()
    {
    }
}
