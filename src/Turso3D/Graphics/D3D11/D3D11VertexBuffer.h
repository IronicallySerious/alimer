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

#pragma once

#include "../../Base/AutoPtr.h"
#include "../../Base/Vector.h"
#include "../Buffer.h"

namespace Turso3D
{
    /// GPU buffer for vertex data.
    class TURSO3D_API VertexBuffer : public Buffer
    {
    public:
        /// Construct.
        VertexBuffer();
        /// Destruct.
        ~VertexBuffer();

        /// Define buffer. Immutable buffers must specify initial data here. Return true on success.
        bool Define(ResourceUsage usage, size_t numVertices, const Vector<VertexElement>& elements, bool useShadowData, const void* data = nullptr);
        /// Define buffer. Immutable buffers must specify initial data here. Return true on success.
        bool Define(ResourceUsage usage, size_t numVertices, size_t numElements, const VertexElement* elements, bool useShadowData, const void* data = nullptr);
        /// Redefine buffer data either completely or partially. Not supported for immutable buffers. Return true on success.
        bool SetData(size_t firstVertex, size_t numVertices, const void* data);

        /// Return CPU-side shadow data if exists.
        unsigned char* ShadowData() const { return shadowData.Get(); }
        /// Return number of vertices.
        size_t NumVertices() const { return numVertices; }
        /// Return number of vertex elements.
        size_t NumElements() const { return elements.Size(); }
        /// Return vertex elements.
        const Vector<VertexElement>& Elements() const { return elements; }
        /// Return size of vertex in bytes.
        size_t VertexSize() const { return vertexSize; }
        /// Return vertex declaration hash code.
        uint32_t ElementHash() const { return elementHash; }

        /// Compute the hash code of one vertex element by index and semantic.
        static uint32_t ElementHash(size_t index, ElementSemantic semantic) { return (semantic + 1) << (index * 3); }

    private:
        /// CPU-side shadow data.
        AutoArrayPtr<unsigned char> shadowData;
        /// Number of vertices.
        size_t numVertices;
        /// Size of vertex in bytes.
        size_t vertexSize;
        /// Vertex elements.
        Vector<VertexElement> elements;
        /// Vertex element hash code.
        uint32_t elementHash;
    };
}
