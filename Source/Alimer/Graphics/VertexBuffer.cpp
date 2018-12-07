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
#include "../Graphics/Graphics.h"
#include "../Debug/Log.h"

namespace Alimer
{
    VertexBuffer::VertexBuffer()
        : GpuBuffer(Object::GetSubsystem<Graphics>())
    {

    }

    bool VertexBuffer::Define(ResourceUsage usage, uint32_t vertexCount, const PODVector<VertexElement>& elements, bool useShadowData, const void* data)
    {
        if (!vertexCount || !elements.Size())
        {
            ALIMER_LOGERROR("Can not define vertex buffer with no vertices or no elements");
            return false;
        }

        return Define(usage, vertexCount, elements.Size(), elements.Data(), useShadowData, data);
    }

    bool VertexBuffer::Define(ResourceUsage usage, uint32_t vertexCount, uint32_t elementsCount, const VertexElement* elements, bool useShadowData, const void* data)
    {
        if (!vertexCount || !elementsCount || !elements)
        {
            ALIMER_LOGERROR("Can not define vertex buffer with no vertices or no elements");
            return false;
        }

        if (usage == ResourceUsage::Immutable && !data)
        {
            ALIMER_LOGERROR("Immutable vertex buffer must define initial data");
            return false;
        }

        Destroy();

        bool useAutoOffset = true;
        for (uint32_t i = 0; i < elementsCount; ++i)
        {
            if (elements[i].offset != 0) 
            {
                useAutoOffset = false;
                break;
            }
        }

        _stride = 0;
        _elements.Resize(elementsCount);
        for (uint32_t i = 0; i < elementsCount; ++i)
        {
            _elements[i] = elements[i];
            _elements[i].offset = useAutoOffset ? _stride : elements[i].offset;
            _stride += GetVertexFormatSize(elements[i].format);
            //elementHash |= ElementHash(i, elements[i].semantic);
        }

        _usage = BufferUsage::Vertex;;
        _size = _stride * vertexCount;
        BufferDescriptor descriptor;
        descriptor.usage = _usage;
        descriptor.size = _size;
        descriptor.stride = _stride;
        return Create(&descriptor, data);
    }
}
