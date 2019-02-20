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

#include "../Graphics/CommandBuffer.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Math/MathUtil.h"
#include "../Core/Log.h"

namespace alimer
{
    CommandBuffer::CommandBuffer(CommandQueue* commandQueue)
        : _commandQueue(commandQueue)
        , _status(CommandBufferStatus::Initial)
    {
        Create();
    }

    CommandBuffer::~CommandBuffer()
    {
        Destroy();
    }

    void CommandBuffer::PushDebugGroup(const std::string& name, const Color4& color)
    {
        PushDebugGroupImpl(name, color);
    }

    void CommandBuffer::PopDebugGroup()
    {
        PopDebugGroupImpl();
    }

    void CommandBuffer::InsertDebugMarker(const std::string& name, const Color4& color)
    {
        InsertDebugMarkerImpl(name, color);
    }

    void CommandBuffer::BeginDefaultRenderPass(const Color4& clearColor, float clearDepth, uint8_t clearStencil)
    {
        _insideRenderPass = true;
        BeginRenderPassImpl(nullptr);
    }

    void CommandBuffer::BeginRenderPass(const RenderPassDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
        _insideRenderPass = true;
        BeginRenderPassImpl(descriptor);
    }

    void CommandBuffer::EndRenderPass()
    {
        EndRenderPassImpl();
        _insideRenderPass = false;
    }

    void CommandBuffer::SetViewport(const RectangleF& viewport)
    {
        //SetViewport(1, &viewport);
    }

    void CommandBuffer::SetScissor(const Rectangle& scissor)
    {
        //SetScissor(1, &scissor);
    }

    void CommandBuffer::SetShader(Shader* shader)
    {
        ALIMER_ASSERT(shader);
        _currentShader = shader;
        //SetShaderImpl(shader);
    }

    void CommandBuffer::SetVertexBuffer(uint32_t binding, Buffer* buffer, uint32_t offset, uint32_t stride, VertexInputRate inputRate)
    {
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT(binding < MaxVertexBufferBindings);

#if defined(ALIMER_DEV)
        if (!any(buffer->GetUsage() & BufferUsage::Vertex))
        {
            //_device->NotifyValidationError("SetVertexBuffer need buffer with Vertex usage");
            return;
        }
#endif
        /*SetVertexBufferImpl(
            binding, 
            buffer,
            offset,
            stride, 
            inputRate);*/
    }

    void CommandBuffer::SetIndexBuffer(Buffer* buffer, uint32_t offset, IndexType indexType)
    {
        ALIMER_ASSERT(buffer);

#if defined(ALIMER_DEV)
        if (!any(buffer->GetUsage() & BufferUsage::Index))
        {
            //_device->NotifyValidationError("SetIndexBuffer need buffer with Index usage");
            return;
        }
#endif

        //SetIndexBufferImpl(buffer, offset, indexType);
    }

    void CommandBuffer::SetPrimitiveTopology(PrimitiveTopology topology)
    {
        //SetPrimitiveTopologyCore(topology);
    }

    void CommandBuffer::Draw(uint32_t vertexCount, uint32_t firstVertex)
    {
        DrawInstanced(vertexCount, 1, firstVertex, 0);
    }

    void CommandBuffer::DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
#if defined(ALIMER_DEV)
        ALIMER_ASSERT(_currentShader && !_currentShader->IsCompute());
        ALIMER_ASSERT_MSG(_insideRenderPass, "Cannot draw outside render pass");
        ALIMER_ASSERT(vertexCount > 0);
        ALIMER_ASSERT(instanceCount >= 1);
#endif

        //DrawInstancedImpl(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void CommandBuffer::DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
    {
        ALIMER_ASSERT(_currentShader && !_currentShader->IsCompute());
        ALIMER_ASSERT(_insideRenderPass);
        ALIMER_ASSERT(indexCount > 1);
        //DrawIndexedImpl(topology, indexCount, startIndexLocation, baseVertexLocation);
    }

    void CommandBuffer::DrawIndexedInstanced(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        ALIMER_ASSERT(_currentShader && !_currentShader->IsCompute());
        ALIMER_ASSERT(_insideRenderPass);
        ALIMER_ASSERT(indexCount > 1);
        //DrawIndexedInstancedImpl(topology, indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void CommandBuffer::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        ALIMER_ASSERT(_currentShader && _currentShader->IsCompute());
        //DispatchImpl(groupCountX, groupCountY, groupCountZ);
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
    void CommandBuffer::BindBuffer(GPUBuffer* buffer, uint32_t set, uint32_t binding)
    {
        ALIMER_ASSERT(buffer);

        BindBuffer(buffer, 0, buffer->GetSize(), set, binding);
    }

    void CommandBuffer::BindBuffer(GPUBuffer* buffer, uint32_t offset, uint32_t range, uint32_t set, uint32_t binding)
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
#endif // TODO

    }