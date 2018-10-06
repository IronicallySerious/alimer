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

#include "../Graphics/CommandBuffer.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/GraphicsImpl.h"
#include "../Math/MathUtil.h"
#include "../Core/Log.h"

namespace Alimer
{
    CommandBuffer::CommandBuffer(GraphicsDevice* graphics, CommandBufferImpl* impl)
        : _graphics(graphics)
        , _impl(impl)
    {
        ALIMER_ASSERT(graphics);
        ALIMER_ASSERT(_impl);
    }

    CommandBuffer::~CommandBuffer()
    {
        SafeDelete(_impl);
    }

    void CommandBuffer::BeginRenderPass(const RenderPassDescriptor* descriptor)
    {
        _impl->BeginRenderPass(descriptor);
    }

    void CommandBuffer::EndRenderPass()
    {
        _impl->EndRenderPass();
    }

    void CommandBuffer::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        _impl->Dispatch(groupCountX, groupCountY, groupCountZ);
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
        ALIMER_ASSERT(any(buffer->GetUsage() & BufferUsage::Vertex));
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
        ALIMER_ASSERT(any(buffer->GetUsage() & BufferUsage::Index));

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
#endif // TODO

}
