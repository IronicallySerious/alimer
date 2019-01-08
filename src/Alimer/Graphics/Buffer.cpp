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

#include "../Graphics/Buffer.h"
#include "../Graphics/GPUDevice.h"
#include "../Graphics/GPUDeviceImpl.h"
#include "../Core/Log.h"

namespace alimer
{
    Buffer::Buffer()
        : GPUResource(GetSubsystem<GPUDevice>(), Type::Buffer)
    {

    }

    Buffer::~Buffer()
    {
        Destroy();
    }

    void Buffer::Destroy()
    {
        SafeDelete(_buffer);
    }

    bool Buffer::CreateGPUBuffer(bool useShadowData, const void* data)
    {
        // Destroy old instance first.
        Destroy();

        // If buffer is reinitialized with the same shadow data, no need to reallocate
        if (useShadowData && (!data || data != _shadowData.Get()))
        {
            _shadowData = new uint8_t[_descriptor.size];
            if (data)
            {
                memcpy(_shadowData.Get(), data, _descriptor.size);
            }
        }

        _buffer = _device->GetImpl()->CreateBuffer(_descriptor, data);
        return _buffer != nullptr;
    }

    bool Buffer::SetSubData(const void* pData)
    {
        return false;
    }

    bool Buffer::SetSubData(uint32_t offset, uint32_t size, const void* pData)
    {
        if (offset + size > GetSize())
        {
            ALIMER_LOGERROR("Buffer subdata out of range");
            return false;
        }

        return false;
    }

    /* VertexBuffer */
    VertexBuffer::VertexBuffer()
    {

    }

    bool VertexBuffer::Define(uint32_t vertexCount, const PODVector<VertexElement>& elements, bool useShadowData, const void* data)
    {
        if (!vertexCount || !elements.Size())
        {
            ALIMER_LOGERROR("Can not define vertex buffer with no vertices or no elements");
            return false;
        }

        return Define(vertexCount, elements.Size(), elements.Data(), useShadowData, data);
    }

    bool VertexBuffer::Define(uint32_t vertexCount, uint32_t elementsCount, const VertexElement* elements, bool useShadowData, const void* data)
    {
        if (!vertexCount || !elementsCount || !elements)
        {
            ALIMER_LOGERROR("Can not define vertex buffer with no vertices or no elements");
            return false;
        }

        _vertexDeclaration.Define(elementsCount, elements);
        _descriptor.size = vertexCount * _vertexDeclaration.GetStride();
        _descriptor.usage = BufferUsage::Vertex;
        _descriptor.stride = _vertexDeclaration.GetStride();
        if (!CreateGPUBuffer(useShadowData, data))
        {
            ALIMER_LOGERROR("Failed to define vertex buffer.");
            return false;
        }

        ALIMER_LOGDEBUG("Defined vertex buffer [vertexCount {}, stride {}]", vertexCount, _descriptor.stride);
        return true;
    }

    /* IndexBuffer */
    IndexBuffer::IndexBuffer()
    {

    }

    bool IndexBuffer::Define(uint32_t indexCount, IndexType indexType, bool useShadowData, const void* data)
    {
        if (!indexCount)
        {
            ALIMER_LOGERROR("Can not define vertex buffer with no indices.");
            return false;
        }

        _indexCount = indexCount;
        _indexType = indexType;
        _descriptor.stride = indexType == IndexType::UInt32 ? 4 : 2;
        _descriptor.size = indexCount * _descriptor.stride;
        _descriptor.usage = BufferUsage::Vertex;
        if (!CreateGPUBuffer(useShadowData, data))
        {
            ALIMER_LOGERROR("Failed to define index buffer.");
            return false;
        }

        ALIMER_LOGDEBUG("Defined index buffer [indexCount {}, indexType {}]", indexCount, indexType);
        return true;
    }
}
