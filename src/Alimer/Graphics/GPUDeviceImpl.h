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
#include "../Graphics/GraphicsDeviceFeatures.h"
#include "../Math/Rectangle.h"

namespace alimer
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

    struct GPUCommandBuffer
    {
        virtual ~GPUCommandBuffer() = 0;

        virtual uint64_t Flush(bool waitForCompletion) = 0;

        virtual void PushDebugGroup(const char* name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(const char* name) = 0;

        virtual void BeginRenderPass(GPUFramebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor) = 0;
        virtual void EndRenderPass() = 0;

        virtual void SetViewport(const RectangleF& viewport) = 0;
        virtual void SetViewport(uint32_t viewportCount, const RectangleF* viewports) = 0;
        virtual void SetScissor(const Rectangle& scissor) = 0;
        virtual void SetScissor(uint32_t scissorCount, const Rectangle* scissors) = 0;
        virtual void SetBlendConstants(const float blendConstants[4]) = 0;

        virtual void SetShader(GPUShader* shader) = 0;

        virtual void SetVertexBuffer(uint32_t binding, GPUBuffer* buffer, const VertexDeclaration* format, uint32_t offset, uint32_t stride, VertexInputRate inputRate) = 0;
        virtual void SetIndexBuffer(GPUBuffer* buffer, uint32_t offset, IndexType indexType) = 0;

        virtual void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
    };

    struct GPUDeviceImpl
    {
        virtual ~GPUDeviceImpl() = 0;

        virtual const GPULimits& GetLimits() const = 0;
        virtual const GraphicsDeviceFeatures& GetFeatures() const = 0;
        virtual GPUCommandBuffer* GetDefaultCommandBuffer() const = 0;

        virtual bool WaitIdle() = 0;
        virtual GPUTexture* CreateTexture(const TextureDescriptor& descriptor, const void* initialData) = 0;
        virtual GPUFramebuffer* CreateFramebuffer() = 0;
        virtual GPUBuffer* CreateBuffer(const BufferDescriptor& descriptor, const void* initialData) = 0;
        virtual GPUShader* CreateComputeShader(const PODVector<uint8_t>& bytecode) = 0;
        virtual GPUShader* CreateGraphicsShader(
            const PODVector<uint8_t>& vertex,
            const PODVector<uint8_t>& tessControl,
            const PODVector<uint8_t>& tessEval,
            const PODVector<uint8_t>& geometry,
            const PODVector<uint8_t>& fragment) = 0;
    };

    inline GPUTexture::~GPUTexture()
    {
    }

    inline GPUBuffer::~GPUBuffer()
    {
    }

    inline GPUFramebuffer::~GPUFramebuffer()
    {
    }

    inline GPUShader::~GPUShader()
    {
    }

    inline GPUCommandBuffer::~GPUCommandBuffer()
    {
    }

    inline GPUDeviceImpl::~GPUDeviceImpl()
    {
    }
}
