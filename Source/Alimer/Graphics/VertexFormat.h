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

#include "../Graphics/Types.h"

namespace Alimer
{
    struct ALIMER_API VertexElement
    {
        /// Default-construct.
        VertexElement() noexcept
            : format(VertexElementFormat::Float3)
            , semantic(VertexElementSemantic::Position)
            , offset(0)
        {
        }

        /// Construct with format, semantic and optional offset.
        VertexElement(VertexElementFormat format_, VertexElementSemantic semantic_, uint32_t offset_ = 0) noexcept
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
        VertexElementFormat format;
        /// Semantic of element.
        VertexElementSemantic semantic;
        /// Offset of element (packed if all elements offset is 0).
        uint32_t offset;
    };

	/// Defines a VertexFormat class.
	class ALIMER_API VertexFormat 
	{
	public:
        /// Constructor.
        VertexFormat(const PODVector<VertexElement>& elements);

        /// Constructor.
        VertexFormat(uint32_t elementsCount, const VertexElement* elements);

        /// Return stride of the format.
        uint32_t GetStride() const { return _stride; }

        /// Return number of vertex elements.
        uint32_t GetElementsCount() const { return _elements.Size(); }

        /// Return vertex elements.
        const PODVector<VertexElement>& GetElements() const { return _elements; }

    private:
        void Initialize(uint32_t elementsCount, const VertexElement* elements);

        PODVector<VertexElement> _elements;
        uint32_t _stride = 0;
	};
}
