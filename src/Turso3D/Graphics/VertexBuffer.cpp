//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "../Debug/Log.h"
#include "../Debug/Profiler.h"
#include "VertexBuffer.h"

#include "../Debug/DebugNew.h"

namespace Turso3D
{
    VertexBuffer::VertexBuffer()
        : Buffer(BufferType::Vertex, ResourceUsage::Default)
    {
    }

    VertexBuffer::~VertexBuffer()
    {
        Release();
    }

    bool VertexBuffer::Define(ResourceUsage usage, uint32_t vertexCount, const Vector<VertexElement>& elements_, bool useShadowData, const void* data)
    {
        if (!vertexCount || !elements_.Size())
        {
            TURSO3D_LOGERROR("Can not define vertex buffer with no vertices or no elements");
            return false;
        }

        return Define(usage, vertexCount, elements_.Size(), &elements_[0], useShadowData, data);
    }

    bool VertexBuffer::Define(ResourceUsage usage, uint32_t vertexCount, size_t numElements_, const VertexElement* elements_, bool useShadowData, const void* data)
    {
        TURSO3D_PROFILE(DefineVertexBuffer);

        if (!vertexCount || !numElements_ || !elements_)
        {
            TURSO3D_LOGERROR("Can not define vertex buffer with no vertices or no elements");
            return false;
        }

        if (usage == ResourceUsage::Immutable && !data)
        {
            TURSO3D_LOGERROR("Immutable vertex buffer must define initial data");
            return false;
        }

        for (size_t i = 0; i < numElements_; ++i)
        {
            if (elements_[i].type >= ELEM_MATRIX3X4)
            {
                TURSO3D_LOGERROR("Matrix elements are not supported in vertex buffers");
                return false;
            }
        }

        _vertexCount = vertexCount;
        _usage = usage;

        // Determine offset of elements and the vertex size & element hash
        _vertexSize = 0;
        _elementHash = 0;
        _elements.Resize(numElements_);
        for (size_t i = 0; i < numElements_; ++i)
        {
            _elements[i] = elements_[i];
            _elements[i].offset = _vertexSize;
            _vertexSize += elementSizes[_elements[i].type];
            _elementHash |= ElementHash(i, _elements[i].semantic);
        }

        _sizeInBytes = _vertexCount * _vertexSize;

        return Create(useShadowData, data);
    }
}
