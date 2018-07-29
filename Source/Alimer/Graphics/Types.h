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

#include "../Util/Util.h"
#include "../Math/Color.h"
#include <array>

namespace Alimer
{
    static constexpr uint32_t MaxViewportsAndScissors = 16u;
    static constexpr uint32_t MaxDescriptorSets = 4u;
    static constexpr uint32_t MaxBindingsPerSet = 16u;
    static constexpr uint32_t MaxVertexAttributes = 16u;
    static constexpr uint32_t MaxVertexBufferBindings = 4u;
    static constexpr uint32_t MaxColorAttachments = 8u;

    /// Enum describing the Graphics backend type.
    enum class GraphicsDeviceType
    {
        /// Best device supported for running platform.
        Default,
        /// Empty/Headless device type.
        Empty,
        /// Vulkan backend.
        Vulkan,
        /// DirectX 11.1+ backend.
        Direct3D11,
        /// DirectX 12 backend.
        Direct3D12,
    };

    /// Enum describing the number of samples.
    enum class SampleCount : uint32_t
    {
        /// 1 sample (no multi-sampling).
        Count1 = 1,
        /// 2 Samples.
        Count2 = 2,
        /// 4 Samples.
        Count4 = 4,
        /// 8 Samples.
        Count8 = 8,
        /// 16 Samples.
        Count16 = 16,
        /// 32 Samples.
        Count32 = 32,
    };

    enum class ResourceUsage : uint32_t
    {
        Default,
        Immutable,
        Dynamic,
        Staging
    };

    /// Primitive topology.
    enum class PrimitiveTopology : uint32_t
    {
        Points,
        Lines,
        LineStrip,
        Triangles,
        TriangleStrip,
        Count
    };

    enum class VertexElementFormat
    {
        Invalid,
        Float,
        Float2,
        Float3,
        Float4,
        Byte4,
        Byte4N,
        UByte4,
        UByte4N,
        Short2,
        Short2N,
        Short4,
        Short4N,
        Count
    };
    
    /// VertexInputRate
    enum class VertexInputRate
    {
        Vertex,
        Instance
    };

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
        VertexElementFormat format;
        /// Offset of element from vertex start.
        uint32_t offset;

        VertexElement() noexcept
            : semanticName(VertexElementSemantic::POSITION)
            , semanticIndex(0)
            , format(VertexElementFormat::Float3)
            , offset(0)
        {
        }

        /// Construct with type, semantic, index and whether is per-instance data.
        VertexElement(
            VertexElementFormat format_,
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


    ALIMER_API uint32_t GetVertexFormatSize(VertexElementFormat format);
}
