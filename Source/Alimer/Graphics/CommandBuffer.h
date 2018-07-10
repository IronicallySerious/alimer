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

#include "../Core/Flags.h"
#include "../Graphics/Commands.h"
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/RenderPass.h"
#include "../Graphics/PipelineState.h"
#include "../Math/Math.h"
#include "../Math/Color.h"

namespace Alimer
{
    class Graphics;

    /// Defines a command buffer for storing recorded gpu commands.
    class ALIMER_API CommandBuffer : public RefCounted
    {
        static constexpr size_t CommandAlign = alignof(Command*);

    public:
        CommandBuffer();

        /// Destructor.
        virtual ~CommandBuffer();

        /// Clear all commands
        void Clear();

        template<typename T>
        void Push(const T& command)
        {
            static_assert(std::is_base_of<Command, T>::value, "Must derive from Command structu");

            size_t offset = _size;
            if (offset % CommandAlign != 0)
            {
                offset += CommandAlign - (offset % CommandAlign);
            }

            if (_capacity < offset + sizeof(T))
            {
                _capacity *= 2;
                if (_capacity < offset + sizeof(T))
                {
                    _capacity = offset + sizeof(T);
                }

                uint8_t* newBuffer = new uint8_t[_capacity];
                if (_buffer)
                {
                    MoveCommands(newBuffer);
                    DeleteCommands();
                    delete[] _buffer;
                }
                _buffer = newBuffer;
            }

            _size = offset + sizeof(T);
            ++_count;
            new (_buffer + offset) T(command);
        }

        Command* Front() const;
        void Pop();

        void BeginRenderPass(RenderPass* renderPass, const Color& clearColor, float clearDepth = 1.0f, uint8_t clearStencil = 0);
        void BeginRenderPass(RenderPass* renderPass, const Rectangle& renderArea, const Color& clearColor, float clearDepth = 1.0f, uint8_t clearStencil = 0);

        void BeginRenderPass(RenderPass* renderPass,
            const Color* clearColors, uint32_t numClearColors,
            float clearDepth = 1.0f, uint8_t clearStencil = 0);

        void BeginRenderPass(RenderPass* renderPass,
            const Rectangle& renderArea,
            const Color* clearColors, uint32_t numClearColors,
            float clearDepth = 1.0f, uint8_t clearStencil = 0);

        void EndRenderPass();

        virtual void SetViewport(const Viewport& viewport);
        virtual void SetViewports(uint32_t numViewports, const Viewport* viewports);

        virtual void SetScissor(const Rectangle& scissor);
        //virtual void SetScissors(uint32_t numScissors, const Rectangle* scissors) = 0;

        void SetVertexBuffer(GpuBuffer* buffer, uint32_t binding, uint64_t offset = 0, VertexInputRate inputRate = VertexInputRate::Vertex);
        void SetIndexBuffer(GpuBuffer* buffer, uint32_t offset = 0, IndexType indexType = IndexType::UInt16);
        //virtual void SetPipeline(const SharedPtr<PipelineState>& pipeline) = 0;

        void SetUniformBuffer(uint32_t set, uint32_t binding, const GpuBuffer* buffer);

        // Draw methods
        void Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount = 1u, uint32_t vertexStart = 0u, uint32_t baseInstance = 0u);
        void DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount = 1u, uint32_t startIndex = 0u);

        void ExecuteCommands(uint32_t commandBufferCount, CommandBuffer* const* commandBuffers);

    protected:
        virtual void BeginRenderPassCore(RenderPass* renderPass, const Rectangle& renderArea, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil);
        virtual void EndRenderPassCore();
        virtual void ExecuteCommandsCore(uint32_t commandBufferCount, CommandBuffer* const* commandBuffers);

        //virtual void DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance) = 0;
        //virtual void DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex) = 0;
        virtual void OnSetVertexBuffer(GpuBuffer* buffer, uint32_t binding, uint64_t offset) {}
        //virtual void SetIndexBufferCore(GpuBuffer* buffer, uint32_t offset, IndexType indexType) = 0;

        inline bool IsInsideRenderPass() const
        {
            return _state == CommandBufferState::InRenderPass;
        }

        inline bool IsOutsideRenderPass() const
        {
            return _state == CommandBufferState::Ready;
        }

        enum class CommandBufferState
        {
            Ready,
            InRenderPass,
            Committed
        };

        CommandBufferState _state = CommandBufferState::Ready;

        enum CommandBufferDirtyBits
        {
            COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT = 1 << 0,
        };
        using CommandBufferDirtyFlags = uint32_t;
        void SetDirty(CommandBufferDirtyFlags flags)
        {
            _dirty |= flags;
        }

        CommandBufferDirtyFlags GetAndClear(CommandBufferDirtyFlags flags)
        {
            auto mask = _dirty & flags;
            _dirty &= ~flags;
            return mask;
        }

        struct VertexBindingState
        {
            GpuBuffer* buffers[MaxVertexBufferBindings];
            uint64_t offsets[MaxVertexBufferBindings];
            uint64_t strides[MaxVertexBufferBindings];
            VertexInputRate inputRates[MaxVertexBufferBindings];
        };

        struct ResourceBindingBufferInfo {
            const GpuBuffer*  buffer;
            uint64_t    offset;
            uint64_t    range;
        };

        struct ResourceBinding
        {
            union {
                ResourceBindingBufferInfo buffer;
            };
        };

        struct ResourceBindings
        {
            ResourceBinding bindings[MaxDescriptorSets][MaxBindingsPerSet];
            uint8_t push_constant_data[MaxDescriptorSets];
        };

        VertexBindingState _vbo = {};
        ResourceBindings _bindings;

        CommandBufferDirtyFlags _dirty = ~0u;
        uint32_t _dirtySets = 0;
        uint32_t _dirtyVbos = 0;

    private:
        template<typename T>
        uint32_t DeleteCommand(T* command)
        {
            ALIMER_UNUSED(command);
            command->~T();
            return sizeof(*command);
        }

        template<typename T>
        uint32_t MoveCommand(T* command, void* newPointer)
        {
            ALIMER_UNUSED(command);
            new (newPointer) T(std::move(*command));
            return sizeof(*command);
        }

        void MoveCommands(uint8_t* newBuffer);
        void DeleteCommands();

    private:
        uint8_t * _buffer = nullptr;
        size_t _capacity = 0;
        size_t _size = 0;
        size_t _position = 0;
        uint32_t _count = 0;
        uint32_t _current = 0;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(CommandBuffer);
    };
}
