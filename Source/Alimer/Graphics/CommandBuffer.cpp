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
#include "../Math/MathUtil.h"
#include "../Core/Log.h"

namespace Alimer
{
    CommandBuffer::CommandBuffer(GraphicsDevice* device, bool secondary)
        : _device(device)
    {
        ALIMER_ASSERT(device);
        BeginCompute();
    }

   
    void CommandBuffer::BeginRenderPass(const RenderPassDescriptor* descriptor)
    {
        BeginRenderPassImpl(descriptor);
        _insideRenderPass = true;
    }

    void CommandBuffer::EndRenderPass()
    {
        EndRenderPassImpl();
        _insideRenderPass = false;
    }

    void CommandBuffer::SetVertexBuffer(VertexBuffer* buffer, uint32_t vertexOffset, VertexInputRate inputRate)
    {
        SetVertexBuffer(0, buffer, vertexOffset, inputRate);
    }

    void CommandBuffer::SetVertexBuffer(uint32_t index, VertexBuffer* buffer, uint32_t vertexOffset, VertexInputRate inputRate)
    {
        ALIMER_ASSERT(index < MaxVertexBufferBindings);
        ALIMER_ASSERT(buffer);

        const uint64_t stride = buffer->GetStride();
        const uint64_t offset = vertexOffset * stride;

        if (_vbo.buffers[index] != buffer
            || _vbo.offsets[index] != vertexOffset)
        {
            _dirtyVbos |= 1u << index;
        }
        
        _vbo.buffers[index] = buffer;
        _vbo.offsets[index] = vertexOffset;
        _vbo.strides[index] = stride;
        _vbo.inputRates[index] = inputRate;
    }

    void CommandBuffer::SetIndexBuffer(IndexBuffer* buffer, uint64_t offset)
    {
        ALIMER_ASSERT(buffer);

        if (_index.buffer == buffer
            && _index.offset == offset)
        {
            return;
        }

        _index.buffer = buffer;
        _index.offset = offset;
        SetIndexBufferImpl(buffer, offset, buffer->GetIndexType());
    }

    void CommandBuffer::SetProgram(Program* program)
    {
        if(_currentProgram == program)
            return;

        _currentProgram = program;
        SetProgramImpl(program);
    }

    void CommandBuffer::SetProgram(const std::string &vertex, const std::string &fragment, const std::vector<std::pair<std::string, int>> &defines)
    {
        ShaderProgram* program = _device->GetShaderManager().RegisterGraphics(vertex, fragment);
        uint32_t variant = program->RegisterVariant(defines);
        SetProgram(program->GetVariant(variant));
    }

    void CommandBuffer::Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
    {
        ALIMER_ASSERT(_currentProgram);
        ALIMER_ASSERT(_insideRenderPass);
        ALIMER_ASSERT(!_isCompute);
        DrawImpl(topology, vertexCount, instanceCount, vertexStart, baseInstance);
    }

    void CommandBuffer::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        //ALIMER_ASSERT(_currentShader);
        ALIMER_ASSERT(_isCompute);
        FlushComputeState();
        DispatchImpl(groupCountX, groupCountY, groupCountZ);
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

    void CommandBuffer::BeginCompute()
    {
        _isCompute = true;
        BeginContext();
    }

    void CommandBuffer::BeginGraphics()
    {
        _isCompute = false;
        BeginContext();
    }

    void CommandBuffer::BeginContext()
    {
        _dirtySets = ~0u;
        _dirtyVbos = ~0u;
        _currentProgram = nullptr;
        memset(&_index, 0, sizeof(_index));
        memset(_vbo.buffers, 0, sizeof(_vbo.buffers));
        _insideRenderPass = false;
    }

    void CommandBuffer::FlushComputeState()
    {

    }


#if TODO
    

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
