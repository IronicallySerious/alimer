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

#include "../Graphics/VertexBuffer.h"
#include "../Graphics/GraphicsImpl.h"
#include "../Util/HashMap.h"

namespace Alimer
{
    uint64_t ElementHash(uint32_t index, const char* semanticName)
    {
        Hasher h;
        h.u32(index);
        h.string(semanticName);
        return h.get();
    }

    VertexBuffer::VertexBuffer()
        : GpuBuffer(nullptr, BufferUsage::Vertex, 0, 0, ResourceUsage::Default)
    {
    }

    VertexBuffer::~VertexBuffer()
    {
        Destroy();
    }

    bool VertexBuffer::Define(uint32_t vertexCount, const std::vector<VertexElement>& elements, ResourceUsage resourceUsage, const void* data)
    {
        if (!vertexCount || !elements.size())
        {
            ALIMER_LOGERROR("Can not define vertex buffer with no vertices or no elements");
            return false;
        }

        return Define(vertexCount,
            static_cast<uint32_t>(elements.size()), elements.data(),
            resourceUsage, data);
    }

    bool VertexBuffer::Define(uint32_t vertexCount, uint32_t numElements, const VertexElement* elements, ResourceUsage resourceUsage, const void* data)
    {
        //ALIMER_PROFILE(DefineVertexBuffer);

        if (!vertexCount || !numElements || !elements)
        {
            ALIMER_LOGERROR("Can not define vertex buffer with no vertices or no elements");
            return false;
        }

        if (resourceUsage == ResourceUsage::Immutable && !data)
        {
            ALIMER_LOGERROR("Immutable vertex buffer must define initial data");
            return false;
        }

        Destroy();
        _vertexCount = vertexCount;
        _resourceUsage = resourceUsage;

        // Determine offset of elements and the vertex size & element hash
        _stride = 0;
        _elementHash = 0;
        _elements.resize(numElements);

        // Check if we need to auto offset.
        bool useAutoOffset = true;
        for (uint32_t i = 0; i < numElements; ++i)
        {
            if (elements[i].offset != 0)
            {
                useAutoOffset = false;
                break;
            }
        }

        uint32_t autoOffset = 0;
        for (uint32_t i = 0; i < numElements; ++i)
        {
            _elements[i] = elements[i];
            autoOffset = _stride;
            if (useAutoOffset)
                _elements[i].offset = _stride;

            _stride += GetVertexFormatSize(elements[i].format);
            _elementHash |= ElementHash(i, elements[i].semanticName);
        }

        _size = _stride * _vertexCount;
        const bool useShadowData = false;
        return GpuBuffer::Create(useShadowData, data);
    }
}
