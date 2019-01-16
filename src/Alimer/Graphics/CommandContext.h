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

#include "../Core/Object.h"
#include "../Graphics/Types.h"
#include "../Graphics/Buffer.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Pipeline.h"
#include "../Math/Rectangle.h"
#include "../Math/Color.h"

namespace alimer
{
    class GPUDevice;
    class GPUCommandBuffer;

    /// Defines a command context for recording gpu commands.
    class ALIMER_API CommandContext final
    {
        friend class GPUDevice;

    private:
        CommandContext(GPUDevice* device, GPUCommandBuffer* commandBuffer);

    public:
        /// Destructor.
        virtual ~CommandContext() = default;

        /// Flush existing commands to the GPU and optionally wait for execution.
        uint64_t Flush(bool waitForCompletion = false);

        void PushDebugGroup(const String& name);
        void PopDebugGroup();
        void InsertDebugMarker(const String& name);

        void BeginRenderPass(Framebuffer* framebuffer, const Color4& clearColor, float clearDepth = 1.0f, uint8_t clearStencil = 0);
        void BeginRenderPass(Framebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor);
        void EndRenderPass();

        void SetViewport(const RectangleF& viewport);
        void SetViewport(uint32_t viewportCount, const RectangleF* viewports);
        void SetScissor(const Rectangle& scissor);
        void SetScissor(uint32_t scissorCount, const Rectangle* scissors);

        void SetBlendColor(const Color4& color);
        void SetBlendColor(float r, float g, float b, float a);

        void SetShader(Shader* shader);

        void SetVertexBuffer(uint32_t binding, Buffer* buffer, uint32_t offset, uint32_t stride, VertexInputRate inputRate = VertexInputRate::Vertex);
        void SetIndexBuffer(Buffer* buffer, uint32_t offset, IndexType indexType);

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

        /// Return the device used for creation.
        GPUDevice* GetDevice() const { return _device; }

    private:
        /// GPUDevice.
        GPUDevice* _device;
        GPUCommandBuffer* _commandBuffer;

        bool _insideRenderPass;
        Shader* _currentShader = nullptr;
    };
}
