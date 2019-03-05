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

#include "../Graphics/CommandBuffer.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Math/MathUtil.h"
#include "../Core/Log.h"

namespace alimer
{
    CommandBuffer::CommandBuffer(GraphicsDevice* device)
        : _device(device)
    {
    }

    void CommandBuffer::Reset()
    {
        _currentShader = nullptr;

        _dirtyVbos = ~0u;
        memset(_currentVertexBuffers, 0, sizeof(_currentVertexBuffers));
    }

    void CommandBuffer::PushDebugGroup(const std::string& name, const Color4& color)
    {
        PushDebugGroupImpl(name, color);
    }

    void CommandBuffer::PopDebugGroup()
    {
        PopDebugGroupImpl();
    }

    void CommandBuffer::InsertDebugMarker(const std::string& name, const Color4& color)
    {
        InsertDebugMarkerImpl(name, color);
    }

    void CommandBuffer::BeginDefaultRenderPass(const Color4& clearColor, float clearDepth, uint8_t clearStencil)
    {
        Texture* texture = _device->GetCurrentTexture();
        Texture* multisampleColorTexture = _device->GetMultisampleColorTexture();
        Texture* depthStencilTexture = _device->GetDepthStencilTexture();

        RenderPassDescriptor renderPass = {};
        renderPass.colorAttachmentCount = 1u; /* TODO: Multisample resolve, is this correct?*/
        if (multisampleColorTexture != nullptr) {
            renderPass.colorAttachments[0].texture = multisampleColorTexture;
            renderPass.colorAttachments[0].resolveTexture = texture;
            renderPass.colorAttachments[0].loadAction = LoadAction::Clear;
            renderPass.colorAttachments[0].storeAction = StoreAction::MultisampleResolve;
            renderPass.colorAttachments[0].clearColor = clearColor;
        }
        else {
            renderPass.colorAttachments[0].texture = texture;
            renderPass.colorAttachments[0].loadAction = LoadAction::Clear;
            renderPass.colorAttachments[0].storeAction = StoreAction::Store;
            renderPass.colorAttachments[0].clearColor = clearColor;
        }

        RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment = {};
        if (depthStencilTexture != nullptr)
        {
            depthStencilAttachment.texture = depthStencilTexture;
            depthStencilAttachment.depthLoadAction = LoadAction::Clear;
            depthStencilAttachment.depthStoreAction = StoreAction::Store;
            depthStencilAttachment.clearDepth = clearDepth;
            if (IsStencilFormat(depthStencilTexture->GetFormat())) {
                depthStencilAttachment.stencilLoadAction = LoadAction::Clear;
                depthStencilAttachment.stencilStoreAction = StoreAction::DontCare;
                depthStencilAttachment.clearStencil = clearStencil;
            }

            renderPass.depthStencilAttachment = &depthStencilAttachment;
        }

        BeginRenderPass(&renderPass);
    }

    void CommandBuffer::BeginRenderPass(const RenderPassDescriptor* renderPass)
    {
        ALIMER_ASSERT_MSG(!_insideRenderPass, "Cannot begin render pass while inside render pass");
        ALIMER_ASSERT(renderPass);

#if defined(ALIMER_DEV)
        // Validate render pass descriptor
        if (renderPass->colorAttachmentCount > MaxColorAttachments) {
            ALIMER_LOGERROR("RenderPassDescriptor color attachments out of bounds");
        }

        for (uint32_t i = 0; i < renderPass->colorAttachmentCount; ++i) {
            Texture* texture = renderPass->colorAttachments[i].texture;
            if (texture == nullptr) {
                ALIMER_LOGERROR("RenderPassDescriptor color attachment texture at index {} is invalid", i);
            }

            if (IsDepthStencilFormat(texture->GetFormat())) {
                ALIMER_LOGERROR("RenderPassDescriptor color attachment texture format at index {} is not color.", i);
                return;
            }

            if (renderPass->colorAttachments[i].storeAction == StoreAction::MultisampleResolve
                || renderPass->colorAttachments[i].storeAction == StoreAction::StoreAndMultisampleResolve) {
                if (renderPass->colorAttachments[i].resolveTexture == nullptr) {
                    ALIMER_LOGERROR("RenderPassDescriptor color attachment resolve texture at index {} is invalid", i);
                }
            }
        }

        if (renderPass->depthStencilAttachment != nullptr) {
            Texture* texture = renderPass->depthStencilAttachment->texture;
            if (texture == nullptr) {
                ALIMER_LOGERROR("RenderPassDescriptor depth attachment texture is invalid");
            }

            if (IsDepthStencilFormat(texture->GetFormat()) == false) {
                ALIMER_LOGERROR("RenderPassDescriptor depth attachment texture format is not depth-stencil.");
                return;
            }
        }

        if (renderPass->colorAttachmentCount == 0 &&
            renderPass->depthStencilAttachment == nullptr) {
            ALIMER_LOGERROR("Cannot use render pass with no attachments.");
        }
#endif
        _insideRenderPass = true;
        BeginRenderPassImpl(renderPass);
    }

    void CommandBuffer::EndRenderPass()
    {
        ALIMER_ASSERT_MSG(_insideRenderPass, "Cannot end render pass if not inside begin render pass");
        EndRenderPassImpl();
        _insideRenderPass = false;
    }

    void CommandBuffer::SetViewport(const RectangleF& viewport)
    {
        SetViewport(1, &viewport);
    }

    void CommandBuffer::SetScissor(const Rectangle& scissor)
    {
        SetScissor(1, &scissor);
    }

    void CommandBuffer::SetShader(Shader* shader)
    {
        ALIMER_ASSERT(shader);
        if (_currentShader == shader)
            return;

        _currentShader = shader;
        SetShaderImpl(shader);
    }

    void CommandBuffer::SetVertexBuffer(uint32_t binding, VertexBuffer* buffer, uint32_t vertexOffset, VertexInputRate inputRate)
    {
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT(binding < MaxVertexBufferBindings);

        if (_currentVertexBuffers[binding].buffer != buffer
            || _currentVertexBuffers[binding].vertexOffset != vertexOffset
            || _currentVertexBuffers[binding].inputRate != inputRate)
        {
            _dirtyVbos |= 1u << binding;
        }

        _currentVertexBuffers[binding].buffer = buffer;
        _currentVertexBuffers[binding].vertexOffset = vertexOffset;
        _currentVertexBuffers[binding].inputRate = inputRate;
    }

    void CommandBuffer::SetIndexBuffer(IndexBuffer* buffer, uint32_t startIndex)
    {
        ALIMER_ASSERT(buffer);
        uint32_t offset = startIndex * buffer->GetIndexSize();
        SetIndexBufferImpl(buffer->GetHandle(), buffer->GetIndexType(), offset);
    }

    void CommandBuffer::Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
#if defined(ALIMER_DEV)
        //ALIMER_ASSERT(_currentShader && !_currentShader->IsCompute());
        ALIMER_ASSERT_MSG(_insideRenderPass, "Cannot draw outside render pass");
        ALIMER_ASSERT(vertexCount > 0);
        ALIMER_ASSERT(instanceCount >= 1);
#endif

        DrawImpl(topology, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void CommandBuffer::DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
    {
#if defined(ALIMER_DEV)
        ALIMER_ASSERT(_currentShader && !_currentShader->IsCompute());
        ALIMER_ASSERT(_insideRenderPass);
        ALIMER_ASSERT(indexCount > 1);
#endif
        DrawIndexedImpl(topology, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    void CommandBuffer::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
#if defined(ALIMER_DEV)
        ALIMER_ASSERT(_currentShader && _currentShader->IsCompute());
#endif
        DispatchImpl(groupCountX, groupCountY, groupCountZ);
    }

    void CommandBuffer::Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX)
    {
        Dispatch(DivideByMultiple(threadCountX, groupSizeX), 1, 1);
    }

    void CommandBuffer::Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX, uint32_t groupSizeY)
    {
        Dispatch(
            DivideByMultiple(threadCountX, groupSizeX),
            DivideByMultiple(threadCountY, groupSizeY),
            1);
    }

    void CommandBuffer::Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ)
    {
        Dispatch(
            DivideByMultiple(threadCountX, groupSizeX),
            DivideByMultiple(threadCountY, groupSizeY),
            DivideByMultiple(threadCountZ, groupSizeZ));
    }

#if TODO
    void CommandBuffer::BindBuffer(GPUBuffer* buffer, uint32_t set, uint32_t binding)
    {
        ALIMER_ASSERT(buffer);

        BindBuffer(buffer, 0, buffer->GetSize(), set, binding);
    }

    void CommandBuffer::BindBuffer(GPUBuffer* buffer, uint32_t offset, uint32_t range, uint32_t set, uint32_t binding)
    {
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT(any(buffer->GetUsage() & BufferUsage::Uniform) || any(buffer->GetUsage() & BufferUsage::Storage));
        ALIMER_ASSERT(set < MaxDescriptorSets);
        ALIMER_ASSERT(binding < MaxBindingsPerSet);

        BindBufferImpl(buffer, offset, range, set, binding);
    }

    void CommandBuffer::BindTexture(Texture* texture, uint32_t set, uint32_t binding)
    {
        ALIMER_ASSERT(texture);
        ALIMER_ASSERT(
            any(texture->GetUsage() & TextureUsage::ShaderRead)
            || any(texture->GetUsage() & TextureUsage::ShaderWrite)
        );
        ALIMER_ASSERT(set < MaxDescriptorSets);
        ALIMER_ASSERT(binding < MaxBindingsPerSet);

        BindTextureImpl(texture, set, binding);
    }
#endif // TODO

}
