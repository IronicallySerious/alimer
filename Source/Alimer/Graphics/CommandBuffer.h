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
#include "../Graphics/VertexBuffer.h"
#include "../Graphics/IndexBuffer.h"
#include "../Graphics/RenderPass.h"
#include "../Graphics/Shader.h"
#include "../Math/Math.h"
#include "../Math/Color.h"

namespace Alimer
{
    class GraphicsDevice;

    /// Defines a command buffer for recording gpu commands.
    class ALIMER_API CommandBuffer : public RefCounted
    {
        friend class GraphicsDevice;

    public:
        CommandBuffer(GraphicsDevice* device, bool secondary);

    public:
        /// Destructor.
        virtual ~CommandBuffer() = default;

        void BeginRenderPass(const RenderPassDescriptor* descriptor);
        void EndRenderPass();

        void SetVertexBuffer(VertexBuffer* buffer, uint32_t vertexOffset = 0, VertexInputRate inputRate = VertexInputRate::Vertex);
        virtual void SetVertexBuffer(uint32_t index, VertexBuffer* buffer, uint32_t vertexOffset = 0, VertexInputRate inputRate = VertexInputRate::Vertex);
        void SetIndexBuffer(IndexBuffer* buffer, uint64_t offset = 0);
        void SetProgram(Program* program);
        void SetProgram(const std::string &vertex, const std::string &fragment, const std::vector<std::pair<std::string, int>> &defines = {});
        void Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t vertexStart = 0, uint32_t baseInstance = 0);

        // Compute
        void Dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
        void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
        void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
        void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);

    protected:
        void BeginCompute();
        void BeginGraphics();
        virtual void BeginContext();
        
        void FlushComputeState();

        // Backend methods.
        virtual void BeginRenderPassImpl(const RenderPassDescriptor* descriptor) = 0;
        virtual void EndRenderPassImpl() = 0;

        virtual void SetProgramImpl(Program* program) = 0;
        virtual void SetIndexBufferImpl(IndexBuffer* buffer, uint64_t offset, IndexType indexType) = 0;
        virtual void DrawImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance) = 0;

        virtual void DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

    protected:
        /// Graphics subsystem.
        GraphicsDevice* _device;
        bool _isCompute;
        bool _insideRenderPass;

        struct VertexBindingState
        {
            VertexBuffer* buffers[MaxVertexBufferBindings];
            uint64_t offsets[MaxVertexBufferBindings];
            uint64_t strides[MaxVertexBufferBindings];
            VertexInputRate inputRates[MaxVertexBufferBindings];
        };

        struct IndexState
        {
            IndexBuffer* buffer;
            uint64_t offset;
        };

        VertexBindingState _vbo = {};
        IndexState _index = {};

        Program* _currentProgram = nullptr;

        uint32_t _dirtySets = 0;
        uint32_t _dirtyVbos = 0;
    };
}
