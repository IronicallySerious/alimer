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
#include "../Graphics/Buffer.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Pipeline.h"
#include "../Math/Rectangle.h"
#include "../Math/Color.h"

namespace alimer
{
    enum class CommandBufferStatus {
        Initial,
        Committed,
        Completed,
        Error
    };

    /// Defines a command buffer for recording gpu commands.
    class ALIMER_API CommandBuffer final : public RefCounted
    {
        friend class CommandQueue;

    private:
        /// Constructor.
        CommandBuffer(CommandQueue* commandQueue);

    public:
        /// Destructor.
        ~CommandBuffer() override;

        void PushDebugGroup(const std::string& name, const Color4& color = Color4::White);
        void PopDebugGroup();
        void InsertDebugMarker(const std::string& name, const Color4& color = Color4::White);

        /// Begin rendering to default backbuffer.
        void BeginDefaultRenderPass(const Color4& clearColor, float clearDepth = 1.0f, uint8_t clearStencil = 0);

        /// Begin pass with given descriptor.
        void BeginRenderPass(const RenderPassDescriptor* descriptor);

        /// End current pass.
        void EndRenderPass();

        void SetViewport(const RectangleF& viewport);
        void SetViewport(uint32_t viewportCount, const RectangleF* viewports);
        void SetScissor(const Rectangle& scissor);
        void SetScissor(uint32_t scissorCount, const Rectangle* scissors);

        void SetBlendColor(const Color4& color);
        void SetStencilReference(uint32_t reference);

        void SetShader(Shader* shader);

        void SetVertexBuffer(uint32_t binding, Buffer* buffer, uint32_t offset, uint32_t stride, VertexInputRate inputRate = VertexInputRate::Vertex);
        void SetIndexBuffer(Buffer* buffer, uint32_t offset, IndexType indexType);

        void SetPrimitiveTopology(PrimitiveTopology topology);

        void Draw(uint32_t vertexCount, uint32_t firstVertex);
        void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

        void DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation);
        void DrawIndexedInstanced(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation);

        // Compute
        void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
        void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
        void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
        void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);

        CommandQueue* GetCommandQueue() const { return _commandQueue; }
        CommandBufferStatus GetStatus() const { return _status; };

#if defined(ALIMER_VULKAN)
        VkCommandBuffer GetHandle() const { return _handle; }
        VkFence         GetFence() const { return _fence; }
#elif defined(ALIMER_D3D12)

#endif

    private:
        // Backend methods
        void Create();
        void Destroy();

        void PushDebugGroupImpl(const std::string& name, const Color4& color);
        void PopDebugGroupImpl();
        void InsertDebugMarkerImpl(const std::string& name, const Color4& color);

        void BeginRenderPassImpl(const RenderPassDescriptor* descriptor);
        void EndRenderPassImpl();

        //virtual void DrawInstancedImpl(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
        //virtual void DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

    private:
        SharedPtr<CommandQueue> _commandQueue;
        CommandBufferStatus _status;
        bool            _insideRenderPass = false;
        Shader*         _currentShader = nullptr;

#if defined(ALIMER_VULKAN)
        void Begin();
        void End();

        VkCommandBuffer _handle = VK_NULL_HANDLE;
        VkFence _fence = VK_NULL_HANDLE;
        bool _supportsDebugUtils = false;
        bool _supportsDebugMarker = false;
#elif defined(ALIMER_D3D12)
#elif defined(ALIMER_D3D11)
#endif
    };
}