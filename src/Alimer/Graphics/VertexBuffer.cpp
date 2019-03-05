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

#include "../Graphics/VertexBuffer.h"
#include "../Graphics/Backend.h"
#include "../Graphics/GraphicsDevice.h"
#include "../foundation/Hash.h"
#include "../Core/Log.h"

namespace alimer
{
    VertexBuffer::VertexBuffer()
        : GPUResource(Type::Buffer)
    {

    }

    VertexBuffer::~VertexBuffer()
    {
        Destroy();
    }

    void VertexBuffer::Destroy()
    {
        SafeDelete(_handle);
    }

    bool VertexBuffer::Create(const void* data)
    {
        if (_device && _device->IsInitialized())
        {
            BufferDescriptor descriptor = {};
            descriptor.usage = BufferUsage::Vertex; /* TODO: Handle Storage and Indirect */
            descriptor.resourceUsage = _usage;
            descriptor.size = _size;
            descriptor.stride = _vertexSize;
            descriptor.cpuAccessible = false;
            _handle = _device->CreateBuffer(&descriptor, data);
        }

        return true;
    }

    bool VertexBuffer::Define(uint32_t vertexCount, const std::vector<VertexElement>& elements, bool useShadowData, ResourceUsage usage, const void* data)
    {
        if (!vertexCount || !elements.size())
        {
            ALIMER_LOGERROR("Can not define vertex buffer with no vertices or no elements");
            return false;
        }

        return Define(vertexCount, elements.size(), elements.data(), useShadowData, usage, data);
    }

    bool VertexBuffer::Define(uint32_t vertexCount, size_t numElements, const VertexElement* elements, bool useShadowData, ResourceUsage usage, const void* data)
    {
        if (!vertexCount || !numElements || !elements)
        {
            ALIMER_LOGERROR("Can not define vertex buffer with no vertices or no elements");
            return false;
        }

        if (usage == ResourceUsage::Immutable
            && !data)
        {
            ALIMER_LOGERROR("Immutable vertex buffer must define initial data");
            return false;
        }

        Destroy();

        _vertexCount = vertexCount;
        _usage = usage;

        bool useAutoOffset = true;
        for (size_t i = 0; i < numElements; ++i)
        {
            if (elements[i].offset != 0)
            {
                useAutoOffset = false;
                break;
            }
        }

        _size = 0;
        _vertexSize = 0;
        _elementsHash = 0;
        _elements.resize(numElements);
        for (size_t i = 0; i < numElements; ++i)
        {
            _elements[i] = elements[i];
            _elements[i].offset = useAutoOffset ? _vertexSize : elements[i].offset;
            _vertexSize += GetVertexElementSize(elements[i].format);
            hash::combine(_elementsHash, i);
            hash::combine(_elementsHash, _elements[i].format);
            hash::combine(_elementsHash, _elements[i].semantic);
            hash::combine(_elementsHash, _elements[i].offset);
        }

        _size = _vertexCount * _vertexSize;

        // If buffer is reinitialized with the same shadow data, no need to reallocate
        if (useShadowData && (!data || data != _shadowData.get()))
        {
            _shadowData.reset(new uint8_t[_size]);
            if (data) {
                memcpy(_shadowData.get(), data, _size);
            }
        }

        return Create(data);
    }
}
