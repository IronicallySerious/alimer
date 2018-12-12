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

#include "../Core/Object.h"
#include "../Graphics/Types.h"
#include "../Graphics/VertexBuffer.h"
#include "../Graphics/IndexBuffer.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Pipeline.h"
#include "../Math/Math.h"
#include "../Math/Color.h"

namespace Alimer
{
    class Graphics;

    struct ColorAttachmentAction {
        LoadAction  loadAction = LoadAction::Clear;
        StoreAction storeAction = StoreAction::Store;
        Color4      clearColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
    };

    struct DepthStencilAttachmentAction {
        LoadAction  depthLoadAction = LoadAction::Clear;
        StoreAction depthStoreAction = StoreAction::DontCare;
        LoadAction  stencilLoadAction = LoadAction::DontCare;
        StoreAction stencilStoreAction = StoreAction::DontCare;
        float       clearDepth = 1.0f;
        uint8_t     clearStencil = 0;
    };

    struct RenderPassBeginDescriptor
    {
        ColorAttachmentAction           colors[MaxColorAttachments];
        DepthStencilAttachmentAction    depthStencil;
    };

    /// Defines a command context for recording gpu commands.
    class ALIMER_API CommandContext : public Object
    {
        ALIMER_OBJECT(CommandContext, Object);

    protected:
        CommandContext(Graphics* graphics);

    public:
        /// Destructor.
        virtual ~CommandContext() = default;

        /// Begin new command context recording.
        static CommandContext& Begin(const String name = String::EMPTY);

        // Flush existing commands to the GPU and optionally wait for execution.
        uint64_t Flush(bool waitForCompletion = false);

        void BeginDefaultRenderPass(const Color4& clearColor, float clearDepth = 1.0f, uint8_t clearStencil = 0);
        void BeginDefaultRenderPass(const RenderPassBeginDescriptor* descriptor);
        void BeginRenderPass(Framebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor);
        void EndRenderPass();

        void SetPipeline(Pipeline* pipeline);

        void SetVertexBuffer(GpuBuffer* buffer, uint32_t offset, uint32_t index);
        void SetVertexBuffers(uint32_t firstBinding, uint32_t count, const GpuBuffer** buffers, const uint32_t* offsets);
        void SetIndexBuffer(IndexBuffer* buffer, uint32_t startIndex = 0);
        //virtual void SetViewport(const rect& viewport) = 0;
        //virtual void SetScissor(const irect& scissor) = 0;

        void SetPrimitiveTopology(PrimitiveTopology topology);

        void Draw(uint32_t vertexCount, uint32_t firstVertex);
        void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

        void DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation);
        void DrawIndexedInstanced(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation);

        // Compute
        void Dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
        void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
        void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
        void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);

    protected:
        void FlushComputeState();

        // Backend methods.
        virtual uint64_t FlushImpl(bool waitForCompletion) = 0;
        //virtual void BeginRenderPassImpl(Framebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor) = 0;
        //virtual void EndRenderPassImpl() = 0;

        //virtual void SetVertexBufferImpl(GpuBuffer* buffer, uint32_t offset) = 0;
        //virtual void SetVertexBuffersImpl(uint32_t firstBinding, uint32_t count, const GpuBuffer** buffers, const uint32_t* offsets) = 0;
        //virtual void SetIndexBufferImpl(IndexBuffer* buffer, uint32_t offset, IndexType indexType) = 0;

        //virtual void SetPrimitiveTopologyImpl(PrimitiveTopology topology) = 0;
        //virtual void DrawImpl(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
        //virtual void DrawInstancedImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) = 0;
        //virtual void DrawIndexedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) = 0;
        //virtual void DrawIndexedInstancedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) = 0;

        void SetName(const String& name) { _name = name; }

    private:
        //virtual void SetPipelineImpl(Pipeline* pipeline) = 0;
        //virtual void DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

    protected:
        /// Graphics subsystem.
        WeakPtr<Graphics> _graphics;

        bool _insideRenderPass;
        Pipeline* _currentPipeline = nullptr;

        uint32_t _dirtySets = 0;
        uint32_t _dirtyVbos = 0;

        String _name;
    };
}
