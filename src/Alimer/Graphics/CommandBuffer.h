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

#include "../Base/Ptr.h"
#include "../Graphics/Types.h"
#include "../Graphics/Pipeline.h"
#include "../Graphics/VertexBuffer.h"
#include "../Graphics/IndexBuffer.h"
#include "../Math/Rectangle.h"
#include "../Math/Color.h"

namespace alimer
{
    struct VertexBufferBinding
    {
        VertexBuffer* buffer;
        uint32_t vertexOffset;
        VertexInputRate inputRate;
    };

    /// Defines a command buffer for recording gpu commands.
    class ALIMER_API CommandBuffer : public RefCounted
    {
    protected:
        /// Constructor.
        CommandBuffer(GraphicsDevice* device);

    public:
        /// Destructor.
        virtual ~CommandBuffer() = default;

        void PushDebugGroup(const std::string& name, const Color4& color = Color4::White);
        void PopDebugGroup();
        void InsertDebugMarker(const std::string& name, const Color4& color = Color4::White);

        /// Begin rendering to default backbuffer.
        void BeginDefaultRenderPass(const Color4& clearColor, float clearDepth = 1.0f, uint8_t clearStencil = 0);

        /// Begin pass with given descriptor.
        void BeginRenderPass(const RenderPassDescriptor* renderPass);

        /// End current pass.
        void EndRenderPass();

        virtual void SetViewport(const RectangleF& viewport);
        virtual void SetViewport(uint32_t viewportCount, const RectangleF* viewports) = 0;
        virtual void SetScissor(const Rectangle& scissor);
        virtual void SetScissor(uint32_t scissorCount, const Rectangle* scissors) = 0;

        virtual void SetBlendColor(const Color4& color) = 0;
        virtual void SetStencilReference(uint32_t reference) = 0;

        void SetPipeline(Pipeline* pipeline);

        void SetVertexBuffer(uint32_t binding, VertexBuffer* buffer, uint32_t vertexOffset = 0, VertexInputRate inputRate = VertexInputRate::Vertex);
        void SetIndexBuffer(IndexBuffer* buffer, uint32_t startIndex = 0);

        void Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
        void DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t baseVertex = 0, uint32_t firstInstance = 0);

        // Compute
        void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
        void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
        void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
        void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);

        GraphicsDevice* GetDevice() const { return _device; }

    protected:
        // Backend methods
        virtual void Reset();
        virtual void PushDebugGroupImpl(const std::string& name, const Color4& color) = 0;
        virtual void PopDebugGroupImpl() = 0;
        virtual void InsertDebugMarkerImpl(const std::string& name, const Color4& color) = 0;

        virtual void BeginRenderPassImpl(const RenderPassDescriptor* renderPass) = 0;
        virtual void EndRenderPassImpl() = 0;

        virtual void SetIndexBufferImpl(BufferHandle* buffer, IndexType indexType, uint32_t offset) = 0;
        virtual void SetPipelineImpl(Pipeline* pipeline) = 0;

        virtual void DrawImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
        virtual void DrawIndexedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) = 0;
        virtual void DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

    protected:
        GraphicsDevice* _device;
        bool            _insideRenderPass = false;
        const Pipeline* _currentPipeline = nullptr;

        uint32_t _dirtyVbos = 0;
        VertexBufferBinding _currentVertexBuffers[MaxVertexBufferBindings] = {};
    };
}
