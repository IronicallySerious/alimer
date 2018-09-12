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
    CommandBuffer::CommandBuffer()
    {
    }

    CommandBuffer::~CommandBuffer()
    {
    }

    bool CommandBuffer::Begin()
    {
        // Don't allow Begin to be called more than once before End is called.
        if (_state == State::Recording)
            return false;

        if (!BeginCore())
            return false;

        // Mark CommandBuffer as recording.
        _state = State::Recording;
        return true;
    }

    bool CommandBuffer::End()
    {
        // Begin must be called first.
        if (_state != State::Recording)
            return false;

        // Mark CommandBuffer as not recording.
        _state = State::None;

        return EndCore();
    }

    void CommandBuffer::EnsureIsRecording()
    {
        if (_state != State::Recording && _state != State::InRenderPass)
        {
            ALIMER_LOGCRITICAL("CommandBuffer is not recording, call Begin first");
        }
    }

    void CommandBuffer::BeginRenderPass(RenderPass* renderPass, const Color4& clearColor, float clearDepth, uint8_t clearStencil)
    {
        BeginRenderPass(renderPass, &clearColor, 1, clearDepth, clearStencil);
    }

    void CommandBuffer::BeginRenderPass(RenderPass* renderPass,
        const Color4* clearColors, uint32_t numClearColors,
        float clearDepth, uint8_t clearStencil)
    {
        EnsureIsRecording();

        if (_state == State::InRenderPass)
        {
            ALIMER_LOGCRITICAL("BeginRenderPass should be called before EndRenderPass.");
        }

        BeginRenderPassCore(renderPass, clearColors, numClearColors, clearDepth, clearStencil);
        _state = State::InRenderPass;
    }

    void CommandBuffer::EndRenderPass()
    {
        if (!IsInsideRenderPass())
        {
            ALIMER_LOGCRITICAL("EndRenderPass must be called inside BeginRenderPass.");
        }

        EndRenderPassCore();
        _state = State::Recording;
    }

    void CommandBuffer::SetShaderProgram(ShaderProgram* program)
    {
        ALIMER_ASSERT(program);
        SetShaderProgramImpl(program);
    }

    void CommandBuffer::BindVertexBuffer(GpuBuffer* buffer, uint32_t binding, GpuSize offset, VertexInputRate inputRate)
    {
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT(buffer->GetUsage() & BufferUsage::Vertex);
        ALIMER_ASSERT(binding < MaxVertexBufferBindings);

        BindVertexBufferImpl(buffer, binding, offset, buffer->GetStride(), inputRate);
    }

    void CommandBuffer::SetVertexInputFormat(VertexInputFormat* format)
    {
        ALIMER_ASSERT(format);

        SetVertexInputFormatImpl(format);
    }

    void CommandBuffer::BindIndexBuffer(GpuBuffer* buffer, GpuSize offset, IndexType indexType)
    {
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT(buffer->GetUsage() & BufferUsage::Index);

        BindIndexBufferImpl(buffer, offset, indexType);
    }

    void CommandBuffer::BindBuffer(GpuBuffer* buffer, uint32_t set, uint32_t binding)
    {
        ALIMER_ASSERT(buffer);

        BindBuffer(buffer, 0, buffer->GetSize(), set, binding);
    }

    void CommandBuffer::BindBuffer(GpuBuffer* buffer, uint32_t offset, uint32_t range, uint32_t set, uint32_t binding)
    {
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT((buffer->GetUsage() & BufferUsage::Uniform) || buffer->GetUsage() & BufferUsage::Storage);
        ALIMER_ASSERT(set < MaxDescriptorSets);
        ALIMER_ASSERT(binding < MaxBindingsPerSet);

        BindBufferImpl(buffer, offset, range, set, binding);
    }

    void CommandBuffer::BindTexture(Texture* texture, uint32_t set, uint32_t binding)
    {
        ALIMER_ASSERT(texture);
        ALIMER_ASSERT((texture->GetUsage() & TextureUsage::ShaderRead) || texture->GetUsage() & TextureUsage::ShaderWrite);
        ALIMER_ASSERT(set < MaxDescriptorSets);
        ALIMER_ASSERT(binding < MaxBindingsPerSet);

        BindTextureImpl(texture, set, binding);
    }

    void CommandBuffer::Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
    {
        if (!IsInsideRenderPass())
        {
            ALIMER_LOGCRITICAL("Cannot draw outside RenderPass.");
        }

        DrawCore(topology, vertexCount, instanceCount, vertexStart, baseInstance);
    }

    void CommandBuffer::DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
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
