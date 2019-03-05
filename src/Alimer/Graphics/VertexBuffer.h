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

#include "../Graphics/GPUResource.h"
#include <vector>

namespace alimer
{
    class BufferHandle;

	/// Defines a GPU VertexBuffer class.
	class ALIMER_API VertexBuffer : public GPUResource, public RefCounted
	{
    public:
        /// Constructor.
        VertexBuffer();
        /// Destructor.
        ~VertexBuffer() override;

        void Destroy() override;

        /// Define buffer. Immutable buffers must specify initial data here. Return true on success.
        bool Define(uint32_t vertexCount, const std::vector<VertexElement>& elements, bool useShadowData = false, ResourceUsage usage = ResourceUsage::Default, const void* data = nullptr);
        /// Define buffer. Immutable buffers must specify initial data here. Return true on success.
        bool Define(uint32_t vertexCount, size_t numElements, const VertexElement* elements, bool useShadowData = false, ResourceUsage usage = ResourceUsage::Default, const void* data = nullptr);

        /// Return number of vertices.
        uint32_t GetVertexCount() const { return _vertexCount; }
        /// Return number of vertex elements.
        uint32_t GetElementsCount() const { return (uint32_t)_elements.size(); }
        /// Return vertex elements.
        const std::vector<VertexElement>& GetElements() const { return _elements; }
        /// Return size of single vertex in bytes.
        uint32_t GetVertexSize() const { return _vertexSize; }
        /// Return vertex declaration hash code.
        size_t GetElementsHash() const { return _elementsHash; }
        /// Return CPU-side shadow data if exists.
        uint8_t* ShadowData() const { return _shadowData.get(); }

        /// Get the backend handle.
        BufferHandle* GetHandle() const { return _handle; }

    private:
        /// Create the GPU-side vertex buffer. Return true on success.
        bool Create(const void* data);

        /// Number of vertices.
        uint32_t _vertexCount = 0;
        std::vector<VertexElement> _elements;
        uint32_t _vertexSize = 0;
        /// Vertex elements hash code.
        size_t _elementsHash = 0;
        /// Size in bytes of buffer.
        uint64_t _size = 0;

        /// Resource usage type.
        ResourceUsage _usage = ResourceUsage::Default;

        /// CPU-side shadow data.
        std::unique_ptr<uint8_t[]> _shadowData;

        BufferHandle* _handle = nullptr;
	};
}
