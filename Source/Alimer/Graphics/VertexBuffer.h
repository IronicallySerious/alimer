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

#pragma once

#include "../Graphics/GpuBuffer.h"

namespace Alimer
{
    struct ALIMER_API VertexElement
    {
        /// Default-construct.
        VertexElement() noexcept
            : format(VertexFormat::Float3)
            , semantic(VertexElementSemantic::Position)
            , offset(0)
        {
        }

        /// Construct with format, semantic and optional offset.
        VertexElement(VertexFormat format_, VertexElementSemantic semantic_, uint32_t offset_ = 0) noexcept
            : format(format_)
            , semantic(semantic_)
            , offset(offset_)
        {
        }

        /// Test for equality with another vertex element. 
        bool operator ==(const VertexElement& rhs) const { return format == rhs.format && semantic == rhs.semantic && offset == rhs.offset; }

        /// Test for inequality with another vertex element.
        bool operator !=(const VertexElement& rhs) const { return !(*this == rhs); }

        /// Format of element.
        VertexFormat format;
        /// Semantic of element.
        VertexElementSemantic semantic;
        /// Offset of element (packed if all elements offset is 0).
        uint32_t offset;
    };

	/// Defines a VertexBuffer class.
	class ALIMER_API VertexBuffer final : public GpuBuffer
	{
	public:
        /// Constructor.
        VertexBuffer();

        /// Define buffer. Immutable buffers must specify initial data here. Return true on success.
        bool Define(ResourceUsage usage, uint32_t vertexCount, const PODVector<VertexElement>& elements, bool useShadowData, const void* data = nullptr);

        /// Define buffer. Immutable buffers must specify initial data here. Return true on success.
        bool Define(ResourceUsage usage, uint32_t vertexCount, uint32_t elementsCount, const VertexElement* elements, bool useShadowData, const void* data = nullptr);

        /// Return number of vertices.
        uint32_t GetVertexCount() const { return _vertexCount; }

        /// Return vertex size (stride( in bytes.
        uint32_t GetVertexSize() const { return _stride; }

        /// Return number of vertex elements.
        uint32_t GetElementsCount() const { return _elements.Size(); }

        /// Return vertex elements.
        const PODVector<VertexElement>& GetElements() const { return _elements; }

    private:
        /// Number of vertices.
        uint32_t _vertexCount = 0;
        /// Vertex elements.
        PODVector<VertexElement> _elements;
	};
}
