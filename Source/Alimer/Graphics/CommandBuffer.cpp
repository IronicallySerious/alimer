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
        Clear();
    }

    CommandBuffer::~CommandBuffer()
    {
        Clear();
    }

    void CommandBuffer::Clear()
    {
        if (_buffer)
        {
            DeleteCommands();
            delete[] _buffer;
        }

        _state = CommandBufferState::Ready;
        _buffer = nullptr;
        _capacity = 0;
        _size = 0;
        _position = 0;
        _count = 0;
        _current = 0;
    }

    Command* CommandBuffer::Front() const
    {
        if (_current >= _count) return nullptr;

        size_t offset = _position;
        if (offset % CommandAlign != 0)
            offset += CommandAlign - (offset % CommandAlign);

        Command* command = reinterpret_cast<Command*>(_buffer + offset);
        return command;
    }

    void CommandBuffer::Pop()
    {
        if (_current >= _count) return;

        if (_position % CommandAlign != 0)
            _position += CommandAlign - (_position % CommandAlign);

        Command* command = reinterpret_cast<Command*>(_buffer + _position);

        switch (command->type)
        {
        case Command::Type::BeginRenderPass:
            _position += DeleteCommand(static_cast<BeginRenderPassCommand*>(command));
            break;

        case Command::Type::EndRenderPass:
            _position += DeleteCommand(static_cast<EndRenderPassCommand*>(command));
            break;

        case Command::Type::SetViewport:
            _position += DeleteCommand(static_cast<SetViewportCommand*>(command));
            break;

        default: break;
        }

        ++_current;

        if (_current == _count)
        {
            _count = 0;
            _size = 0;
            _position = 0;
            _current = 0;
        }
    }

    void CommandBuffer::MoveCommands(uint8_t* newBuffer)
    {
        size_t offset = _position;

        for (uint32_t i = _current; i < _count; ++i)
        {
            if (offset % CommandAlign != 0)
                offset += CommandAlign - (offset % CommandAlign);

            Command* command = reinterpret_cast<Command*>(_buffer + offset);

            switch (command->type)
            {
            case Command::Type::BeginRenderPass:
                offset += MoveCommand(static_cast<BeginRenderPassCommand*>(command), newBuffer + offset);
                break;

            case Command::Type::EndRenderPass:
                offset += MoveCommand(static_cast<EndRenderPassCommand*>(command), newBuffer + offset);
                break;

            case Command::Type::SetViewport:
                offset += MoveCommand(static_cast<SetViewportCommand*>(command), newBuffer + offset);
                break;

            default: assert(false);
            }
        }
    }

    void CommandBuffer::DeleteCommands()
    {
        size_t offset = _position;

        for (uint32_t i = _current; i < _count; ++i)
        {
            if (offset % CommandAlign != 0)
                offset += CommandAlign - (offset % CommandAlign);

            Command* command = reinterpret_cast<Command*>(_buffer + offset);

            switch (command->type)
            {
            case Command::Type::BeginRenderPass:
                offset += DeleteCommand(static_cast<BeginRenderPassCommand*>(command));
                break;

            case Command::Type::EndRenderPass:
                offset += DeleteCommand(static_cast<EndRenderPassCommand*>(command));
                break;

            case Command::Type::SetViewport:
                offset += DeleteCommand(static_cast<SetViewportCommand*>(command));
                break;

            default: assert(false);
            }
        }
    }

    void CommandBuffer::BeginRenderPass(RenderPass* renderPass, const Color& clearColor, float clearDepth, uint8_t clearStencil)
    {
        BeginRenderPass(renderPass, Rectangle::Empty, &clearColor, 1, clearDepth, clearStencil);
    }

    void CommandBuffer::BeginRenderPass(RenderPass* renderPass,
        const Rectangle& renderArea,
        const Color& clearColor,
        float clearDepth, uint8_t clearStencil)
    {
        BeginRenderPass(renderPass, renderArea, &clearColor, 1, clearDepth, clearStencil);
    }

    void CommandBuffer::BeginRenderPass(RenderPass* renderPass,
        const Color* clearColors, uint32_t numClearColors,
        float clearDepth, uint8_t clearStencil)
    {
        BeginRenderPass(renderPass, Rectangle::Empty, clearColors, numClearColors, clearDepth, clearStencil);
    }

    void CommandBuffer::BeginRenderPass(RenderPass* renderPass,
        const Rectangle& renderArea,
        const Color* clearColors, uint32_t numClearColors,
        float clearDepth, uint8_t clearStencil)
    {
        if (!IsOutsideRenderPass())
        {
            ALIMER_LOGCRITICAL("BeginRenderPass should be called before EndRenderPass.");
        }

        BeginRenderPassCore(renderPass, renderArea, clearColors, numClearColors, clearDepth, clearStencil);
        _state = CommandBufferState::InRenderPass;
    }

    void CommandBuffer::EndRenderPass()
    {
        if (!IsInsideRenderPass())
        {
            ALIMER_LOGCRITICAL("EndRenderPass must be called inside BeginRenderPass.");
        }

        EndRenderPassCore();
        _state = CommandBufferState::Ready;
    }

    void CommandBuffer::SetViewport(const Viewport& viewport)
    {
        Push(SetViewportCommand(viewport));
    }

    void CommandBuffer::SetViewports(uint32_t numViewports, const Viewport* viewports)
    {
        Push(SetViewportsCommand(numViewports, viewports));
    }

    void CommandBuffer::SetScissor(const Rectangle& scissor)
    {
        //SetScissors(1, &scissor);
    }

    void CommandBuffer::SetShader(Shader* shader)
    {
        ALIMER_ASSERT(shader);
        SetShaderCore(shader);
    }

    void CommandBuffer::SetShaderCore(Shader* shader)
    {
    }

    void CommandBuffer::SetVertexBuffer(uint32_t binding, VertexBuffer* buffer, uint64_t offset, VertexInputRate inputRate)
    {
        ALIMER_ASSERT(binding < MaxVertexBufferBindings);
        ALIMER_ASSERT(buffer);

        SetVertexBufferCore(binding, buffer, offset, buffer->GetStride(), inputRate);
    }

    void CommandBuffer::SetVertexBufferCore(uint32_t binding, VertexBuffer* buffer, uint64_t offset, uint64_t stride, VertexInputRate inputRate)
    {

    }

    void CommandBuffer::SetIndexBuffer(GpuBuffer* buffer, uint32_t offset)
    {
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT(buffer->GetUsage() & BufferUsage::Index);

        SetIndexBufferCore(buffer, offset, buffer->GetStride() == 2 ? IndexType::UInt16 : IndexType::UInt32);
    }

    void CommandBuffer::SetIndexBufferCore(GpuBuffer* buffer, uint32_t offset, IndexType indexType)
    {

    }

    void CommandBuffer::SetUniformBuffer(uint32_t set, uint32_t binding, GpuBuffer* buffer)
    {
        ALIMER_ASSERT(set < MaxDescriptorSets);
        ALIMER_ASSERT(binding < MaxBindingsPerSet);
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT(buffer->GetUsage() & BufferUsage::Uniform);

        SetUniformBufferCore(set, binding, buffer, 0, buffer->GetSize());
    }

    void CommandBuffer::SetUniformBufferCore(uint32_t set, uint32_t binding, GpuBuffer* buffer, uint64_t offset, uint64_t range)
    {
    }

    void CommandBuffer::SetTexture(uint32_t binding, Texture* texture, ShaderStageFlags stage)
    {
        ALIMER_ASSERT(binding < MaxBindingsPerSet);
        SetTextureCore(binding, texture, stage);
    }

    void CommandBuffer::SetTextureCore(uint32_t binding, Texture* texture, ShaderStageFlags stage)
    {

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

    void CommandBuffer::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {

    }

    void CommandBuffer::ExecuteCommands(uint32_t commandBufferCount, CommandBuffer* const* commandBuffers)
    {
        ALIMER_ASSERT(commandBufferCount);
        ALIMER_ASSERT(commandBuffers);

        ExecuteCommandsCore(commandBufferCount, commandBuffers);
    }

    void CommandBuffer::BeginRenderPassCore(RenderPass* renderPass, const Rectangle& renderArea, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil)
    {
        Push(BeginRenderPassCommand(renderPass, renderArea, clearColors, numClearColors, clearDepth, clearStencil));
    }

    void CommandBuffer::EndRenderPassCore()
    {
        Push(EndRenderPassCommand());
    }

    void CommandBuffer::ExecuteCommandsCore(uint32_t commandBufferCount, CommandBuffer* const* commandBuffers)
    {

    }
}
