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

#include "Graphics/CommandBuffer.h"
#include "Graphics/Graphics.h"
#include "../Core/Log.h"
#include "../Math/MathUtil.h"

namespace Alimer
{
    CommandContext::CommandContext()
    {
    }

    CommandContext::~CommandContext()
    {
    }

    void CommandContext::BeginRenderPass(RenderPass* renderPass, const Color& clearColor, float clearDepth, uint8_t clearStencil)
    {
        BeginRenderPass(renderPass, &clearColor, 1, clearDepth, clearStencil);
    }

    void CommandContext::BeginRenderPass(RenderPass* renderPass,
        const Color* clearColors, uint32_t numClearColors,
        float clearDepth, uint8_t clearStencil)
    {
        if (!IsOutsideRenderPass())
        {
            ALIMER_LOGCRITICAL("BeginRenderPass should be called before EndRenderPass.");
        }

        BeginRenderPassCore(renderPass, clearColors, numClearColors, clearDepth, clearStencil);
        _state = CommandBufferState::InRenderPass;
    }

    void CommandContext::EndRenderPass()
    {
        if (!IsInsideRenderPass())
        {
            ALIMER_LOGCRITICAL("EndRenderPass must be called inside BeginRenderPass.");
        }

        EndRenderPassCore();
        _state = CommandBufferState::Ready;
    }

    void CommandContext::SetShader(Shader* shader)
    {
        ALIMER_ASSERT(shader);
        SetShaderCore(shader);
    }

    void CommandContext::SetVertexBuffer(uint32_t binding, VertexBuffer* buffer, uint64_t offset, VertexInputRate inputRate)
    {
        ALIMER_ASSERT(binding < MaxVertexBufferBindings);
        ALIMER_ASSERT(buffer);

        SetVertexBufferCore(binding, buffer, offset, buffer->GetStride(), inputRate);
    }

    void CommandContext::SetIndexBuffer(GpuBuffer* buffer, uint32_t offset)
    {
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT(buffer->GetUsage() & BufferUsage::Index);

        SetIndexBufferCore(buffer, offset, buffer->GetStride() == 2 ? IndexType::UInt16 : IndexType::UInt32);
    }

    void CommandContext::SetUniformBuffer(uint32_t set, uint32_t binding, GpuBuffer* buffer)
    {
        ALIMER_ASSERT(set < MaxDescriptorSets);
        ALIMER_ASSERT(binding < MaxBindingsPerSet);
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT(buffer->GetUsage() & BufferUsage::Uniform);

        SetUniformBufferCore(set, binding, buffer, 0, buffer->GetSize());
    }

    void CommandContext::SetTexture(uint32_t binding, Texture* texture, ShaderStageFlags stage)
    {
        ALIMER_ASSERT(binding < MaxBindingsPerSet);
        SetTextureCore(binding, texture, stage);
    }

    void CommandContext::Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
    {
        if (!IsInsideRenderPass())
        {
            ALIMER_LOGCRITICAL("Cannot draw outside RenderPass.");
        }

        DrawCore(topology, vertexCount, instanceCount, vertexStart, baseInstance);
    }

    void CommandContext::DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {
        if (!IsInsideRenderPass())
        {
            ALIMER_LOGCRITICAL("Cannot draw outside RenderPass.");
        }

        DrawIndexedCore(topology, indexCount, instanceCount, startIndex);
    }

    /*void CommandBuffer::ExecuteCommands(uint32_t commandBufferCount, CommandBuffer* const* commandBuffers)
    {
        ALIMER_ASSERT(commandBufferCount);
        ALIMER_ASSERT(commandBuffers);

        ExecuteCommandsCore(commandBufferCount, commandBuffers);
    }*/
}
