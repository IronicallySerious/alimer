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
    VertexBuffer::VertexBuffer(Graphics* graphics, const VertexFormat& vertexFormat, uint32_t vertexCount, ResourceUsage resourceUsage, const void* initialData)
        : GpuBuffer(graphics, BufferUsage::Vertex, resourceUsage)
        , _vertexFormat(vertexFormat)
        , _vertexCount(vertexCount)
    {
        if (!vertexCount || !vertexFormat.GetElementsCount())
        {
            ALIMER_LOGERROR("Can not define vertex buffer with no vertices or no elements");
            return;
        }

        if (resourceUsage == ResourceUsage::Immutable
            && !initialData)
        {
            ALIMER_LOGERROR("Immutable vertex buffer must define initial data");
            return;
        }

        // Determine offset of elements and the vertex size & element hash
        _vertexFormat = vertexFormat;
        _stride = vertexFormat.GetStride();
        _size = _stride * _vertexCount;
        Create(initialData);
    }

    VertexBuffer::~VertexBuffer()
    {
        Destroy();
    }
}
