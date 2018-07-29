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
#include <vector>

namespace Alimer
{
    /// Defines a GPU VertexFormat.
    class VertexFormat final
    {
    public:
        /// Constructor.
        VertexFormat(const std::vector<VertexElement>& elements);

        /// Constructor.
        VertexFormat(const VertexElement* elements, size_t elementCount);

        /// Destructor.
        ~VertexFormat();

        /// Return number of vertex elements.
        uint32_t GetElementsCount() const { return static_cast<uint32_t>(_elements.size()); }

        /// Return vertex elements.
        const std::vector<VertexElement>& GetElements() const { return _elements; }

        /// Return the number of bytes from one vertex to the next.
        uint32_t GetStride() const { return _stride; }

        /// Return vertex declaration hash code.
        uint64_t GetHash() const { return _hash; }

    private:
        void Initialize(const VertexElement* elements, size_t elementCount);

        /// Vertex elements.
        std::vector<VertexElement> _elements;

        /// VertexFormat stride.
        uint32_t _stride;

        /// Vertex element hash code.
        uint64_t _hash;
    };
}
