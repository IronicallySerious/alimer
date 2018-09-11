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

#include "../Core/Flags.h"
#include "../Util/Util.h"
#include "../Math/Math.h"
#include "../Math/Color.h"
#include <string>

namespace Alimer
{
    static constexpr uint32_t MaxViewportsAndScissors = 16u;
    static constexpr uint32_t MaxDescriptorSets = 4u;
    static constexpr uint32_t MaxBindingsPerSet = 14u;
    static constexpr uint32_t MaxVertexAttributes = 16u;
    static constexpr uint32_t MaxVertexBufferBindings = 4u;
    static constexpr uint32_t MaxColorAttachments = 8u;

    using GpuSize = uint64_t;

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

    enum class BufferUsage : uint32_t
    {
        None = 0,
        TransferSrc = 1 << 0,
        TransferDest = 1 << 1,
        Vertex = 1 << 2,
        Index = 1 << 3,
        Uniform = 1 << 4,
        Storage = 1 << 5,
        Indirect = 1 << 6,
    };

    using BufferUsageFlags = Flags<BufferUsage, uint32_t>;
    ALIMER_FORCE_INLINE BufferUsageFlags operator|(BufferUsage bit0, BufferUsage bit1)
    {
        return BufferUsageFlags(bit0) | bit1;
    }

    ALIMER_FORCE_INLINE BufferUsageFlags operator~(BufferUsage bits)
    {
        return ~(BufferUsageFlags(bits));
    }

    /// Defines shader stage
    enum class ShaderStage : uint32_t
    {
        Vertex = 0,
        TessControl = 1,
        TessEvaluation = 2,
        Geometry = 3,
        Fragment = 4,
        Compute = 5,
        Count
    };

    /// Defines shader stage usage.
    enum class ShaderStageUsage : uint32_t
    {
        None = 0,
        Vertex = 1 << 0,
        TessControl = 1 << 1,
        TessEvaluation = 1 << 2,
        Geometry = 1 << 3,
        Fragment = 1 << 4,
        Compute = 1 << 5,
        AllGraphics = (Vertex | TessControl | TessEvaluation | Geometry | Fragment),
        All = (AllGraphics | Compute),
    };

    using ShaderStageFlags = Flags<ShaderStageUsage>;
    ALIMER_FORCE_INLINE ShaderStageFlags operator|(ShaderStageUsage bit0, ShaderStageUsage bit1)
    {
        return ShaderStageFlags(bit0) | bit1;
    }

    ALIMER_FORCE_INLINE ShaderStageFlags operator~(ShaderStageUsage bits)
    {
        return ~(ShaderStageFlags(bits));
    }

    enum class VertexFormat : uint32_t
    {
        Invalid = 0,
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

    enum class ParamDataType
    {
        Unknown = 0,
        Void,
        Boolean,
        Char,
        Int,
        UInt,
        Int64,
        UInt64,
        Half,
        Float,
        Double,
        Struct
    };

    enum class ParamAccess
    {
        Read,
        Write,
        ReadWrite
    };

    enum class ResourceParamType
    {
        Input = 0,
        Output = 1,
    };

    enum class IndexType : uint32_t
    {
        UInt16,
        UInt32,
    };

    struct BufferDescriptor
    {
        /// Buffer usage.
        BufferUsageFlags usage = BufferUsage::None;

        /// Size in bytes of buffer.
        uint64_t size = 0;

        /// Size of each individual element in the buffer, in bytes. 
        uint32_t stride = 0;
    };

    struct PipelineResource
    {
        std::string name;
        ShaderStageFlags stages;
        ResourceParamType resourceType;
        ParamDataType dataType;
        ParamAccess access;
        uint32_t set;
        uint32_t binding;
        uint32_t location;
        uint32_t vecSize;
        uint32_t arraySize;
        uint32_t offset;
        uint32_t size;
    };

    class ShaderModule;
    struct ShaderStageDescriptor
    {
        ShaderModule* module = nullptr;
        std::string entryPoint = "main";
        // TODO: Add specialization info.
        //const VkSpecializationInfo* pSpecializationInfo;
    };

    struct ShaderProgramDescriptor
    {
        uint32_t stageCount;
        const ShaderStageDescriptor* stages;
    };

    struct Viewport
    {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        float minDepth = 0.0f;
        float maxDepth = 1.0f;

        Viewport() = default;
        constexpr Viewport(float x_, float y_, float width_, float height_, float minDepth_ = 0.0f, float maxDepth_ = 1.0f)
            : x(x_), y(y_), width(width_), height(height_), minDepth(minDepth_), maxDepth(maxDepth_) {}

        explicit Viewport(const Rectangle& rect)
            : x(static_cast<float>(rect.x)), y(static_cast<float>(rect.y))
            , width(static_cast<float>(rect.width)), height(static_cast<float>(rect.height))
            , minDepth(0.0f), maxDepth(1.0f) {}

        Viewport(const Viewport&) = default;
        Viewport& operator=(const Viewport&) = default;

        Viewport(Viewport&&) = default;
        Viewport& operator=(Viewport&&) = default;

        // Comparison operators
        bool operator == (const Viewport& rhs) const
        {
            return (x == rhs.x && y == rhs.y
                && width == rhs.width && height == rhs.height
                && minDepth == rhs.minDepth && maxDepth == rhs.maxDepth);
        }

        bool operator != (const Viewport& rhs) const
        {
            return (x != rhs.x || y != rhs.y
                || width != rhs.width || height != rhs.height
                || minDepth != rhs.minDepth || maxDepth != rhs.maxDepth);
        }

        // Assignment operators
        Viewport& operator= (const Rectangle& rect)
        {
            x = static_cast<float>(rect.x);
            y = static_cast<float>(rect.y);
            width = static_cast<float>(rect.width);
            height = static_cast<float>(rect.height);
            minDepth = 0.0f; maxDepth = 1.0f;
            return *this;
        }

        /// Get aspect ratio.
        inline float GetAspectRatio() const
        {
            if (width == 0.0f || height == 0.0f)
                return 0.0f;

            return (width / height);
        }
    };

    ALIMER_API uint32_t GetVertexFormatSize(VertexFormat format);
}
