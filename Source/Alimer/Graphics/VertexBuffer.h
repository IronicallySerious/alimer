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
#include <vector>

namespace Alimer
{
    /// Element semantics for vertex elements.
    class ALIMER_API VertexElementSemantic
    {
    public:
        static constexpr const char* POSITION = "POSITION";
        static constexpr const char* NORMAL = "NORMAL";
        static constexpr const char* BINORMAL = "BINORMAL";
        static constexpr const char* TANGENT = "TANGENT";
        static constexpr const char* TEXCOORD = "TEXCOORD";
        static constexpr const char* COLOR = "COLOR";
        static constexpr const char* BLENDWEIGHT = "BLENDWEIGHT";
        static constexpr const char* BLENDINDICES = "BLENDINDICES";
    };

    class ALIMER_API VertexElement
    {
    public:
        /// Semantic of element.
        const char* semanticName;
        /// Semantic index of element, for example multi-texcoords.
        uint32_t semanticIndex = 0;
        /// Format of element.
        VertexFormat format;
        /// Offset of element from vertex start.
        uint32_t offset;

        VertexElement() noexcept
            : semanticName(VertexElementSemantic::POSITION)
            , semanticIndex(0)
            , format(VertexFormat::Float3)
            , offset(0)
        {
        }

        /// Construct with type, semantic, index and whether is per-instance data.
        VertexElement(
            VertexFormat format_,
            const char* semanticName_,
            uint32_t semanticIndex_ = 0,
            uint32_t offset_ = 0)
            : format(format_)
            , semanticName(semanticName_)
            , semanticIndex(semanticIndex_)
            , offset(offset_)
        {
        }

        VertexElement(const VertexElement&) = default;
        VertexElement& operator=(const VertexElement&) = default;

        VertexElement(VertexElement&&) = default;
        VertexElement& operator=(VertexElement&&) = default;
    };

    /// Defines a GPU VertexBuffer class.
    class VertexBuffer final : public GpuBuffer
    {
    public:
        /// Constructor.
        VertexBuffer(Graphics* graphics);

        /// Destructor.
        virtual ~VertexBuffer() override;

        /// Define buffer. Immutable buffers must specify initial data here. Return true on success.
        bool Define(uint32_t vertexCount, const std::vector<VertexElement>& elements, ResourceUsage resourceUsage = ResourceUsage::Default, const void* data = nullptr);

        /// Define buffer. Immutable buffers must specify initial data here. Return true on success.
        bool Define(uint32_t vertexCount, uint32_t numElements, const VertexElement* elements, ResourceUsage resourceUsage = ResourceUsage::Default, const void* data = nullptr);

        /// Return number of vertices.
        uint32_t GetVertexCount() const { return _vertexCount; }

        /// Return number of vertex elements.
        uint32_t GetElementsCount() const { return static_cast<uint32_t>(_elements.size()); }
        /// Return vertex elements.
        const std::vector<VertexElement>& GetElements() const { return _elements; }
        /// Return vertex declaration hash code.
        uint64_t GetElementHash() const { return _elementHash; }

    private:
        uint32_t _vertexCount = 0;

        /// Vertex elements.
        std::vector<VertexElement> _elements;

        /// Vertex element hash code.
        uint64_t _elementHash = 0;
    };
}
