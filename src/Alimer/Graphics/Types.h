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

#include <foundation/cpp_macros.h>
#include "../Base/String.h"
#include "../Math/Math.h"
#include "../Math/Color.h"
#include "../Graphics/PixelFormat.h"
#include "fmt/format.h"

namespace alimer
{
    /* Constants */
    static constexpr uint32_t RemainingMipLevels = ~0U;
    static constexpr uint32_t RemainingArrayLayers = ~0U;
    static constexpr uint32_t MaxViewportsAndScissors = 16u;
    static constexpr uint32_t MaxDescriptorSets = 4u;
    static constexpr uint32_t MaxBindingsPerSet = 14u;
    static constexpr uint32_t MaxVertexAttributes = 16u;
    static constexpr uint32_t MaxVertexBufferBindings = 4u;
    static constexpr uint32_t MaxColorAttachments = 8u;

    /* Enums */

    /// Enum describing the Graphics backend.
    enum class GraphicsBackend : uint32_t
    {
        /// Best device supported for running platform.
        Default,
        /// Vulkan backend.
        Vulkan,
        /// Direct3D 11 backend.
        D3D11,
        /// Direct3D 12 backend.
        D3D12,
        /// OpenGL backend.
        OpenGL,
        /// Null backend.
        Null,
        /// Count
        Count
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

    enum class ResourceUsage : unsigned
    {
        Default,
        Immutable,
        Dynamic,
        Staging
    };

    enum class LoadAction
    {
        DontCare,
        Load,
        Clear
    };

    enum class StoreAction
    {
        DontCare,
        Store
    };

    /// Primitive topology.
    enum class PrimitiveTopology : uint32_t
    {
        PointList = 0,
        LineList = 1,
        LineStrip = 2,
        TriangleList = 3,
        TriangleStrip = 4,
        LineListWithAdjacency = 5,
        LineStripWithAdjacency = 6,
        TriangleListWithAdjacency = 7,
        TriangleStripWithAdjacency = 8,
        PatchList = 9,
        Count = 10,
    };

    enum class VertexFormat : uint32_t
    {
        Unknown = 0,
        Float = 1,
        Float2 = 2,
        Float3 = 3,
        Float4 = 4,
        Byte4 = 5,
        Byte4N = 6,
        UByte4 = 7,
        UByte4N = 8,
        Short2 = 9,
        Short2N = 10,
        Short4 = 11,
        Short4N = 12,
        Count = 13
    };

    /// VertexInputRate
    enum class VertexInputRate : uint32_t
    {
        Vertex = 0,
        Instance = 1
    };

    enum class IndexType : uint32_t
    {
        UInt16 = 0,
        UInt32 = 1
    };

    enum class CompareFunction : uint32_t
    {
        Never = 0,
        Less = 1,
        Equal = 2,
        LessEqual = 3,
        Greater = 4,
        NotEqual = 5,
        GreaterEqual = 6,
        Always = 7
    };

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

    /// Defines shader stages.
    enum class ShaderStages : uint32_t
    {
        None = 0,
        Vertex = 0x00000001,
        TessellationControl = 0x00000002,
        TessellationEvaluation = 0x00000004,
        Geometry = 0x00000008,
        Fragment = 0x00000010,
        Compute = 0x00000020,
        AllGraphics = 0x0000001F,
        All = 0x7FFFFFFF,
    };
    ALIMER_BITMASK(ShaderStages);

    /// Texture types.
    enum class TextureType : uint32_t
    {
        Type1D,
        Type2D,
        TypeCube,
        Type3D
    };

    /// Defines texture usage enum.
    enum class TextureUsage : uint32_t
    {
        None = 0,
        /// Specifies that the buffer can be used as the source of a transfer command
        TransferSrc     = 1 << 0,
        /// Specifies that the buffer can be used as the destination of a transfer command.
        TransferDest    = 1 << 1,
        /// Specifies that the image can be used for reading or sampling from the shader.
        Sampled         = 1 << 2,
        /// Specifies that the image can be written from shader.
        Storage         = 1 << 3,
        /// Specifies that the image can be used as a color, depth, or stencil render target in a render pass descriptor.
        RenderTarget    = 1 << 4,
    };
    ALIMER_BITMASK(TextureUsage);

    /* Sampler */
    enum class SamplerAddressMode : uint32_t
    {
        /// Texture coordinates wrap to the other side of the texture, effectively keeping only the fractional part of the texture coordinate.
        Repeat,
        /// Between -1.0 and 1.0, the texture coordinates are mirrored across the axis. Outside -1.0 and 1.0, the image is repeated.
        MirrorRepeat,
        /// Texture coordinates are clamped between 0.0 and 1.0, inclusive.
        ClampToEdge,
        /// Out-of-range texture coordinates return the value specified by the sampler's border color.
        ClampToBorder,
        /// Between -1.0 and 1.0, the texture coordinates are mirrored across the axis. Outside -1.0 and 1.0, the texture coordinates are clamped.
        MirrorClampToEdge
    };

    enum class SamplerMinMagFilter
    {
        Nearest,
        Linear,
    };

    enum class SamplerMipFilter
    {
        Nearest,
        Linear,
    };

    enum class SamplerBorderColor
    {
        TransparentBlack,
        OpaqueBlack,
        OpaqueWhite
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

    struct PipelineResource
    {
        String name;
        ShaderStages stages;
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

    enum class VertexElementSemantic : uint32_t
    {
        Position = 0,
        Normal,
        Binormal,
        Tangent,
        BlendWeight,
        BlendIndices,
        Color0,
        Color1,
        Color2,
        Color3,
        Texcoord0,
        Texcoord1,
        Texcoord2,
        Texcoord3,
        Texcoord4,
        Texcoord5,
        Texcoord6,
        Texcoord7,
        Count
    };

    enum class BufferUsage : uint32_t
    {
        None = 0,
        Vertex = 1 << 0,
        Index = 1 << 1,
        Uniform = 1 << 2,
        Storage = 1 << 3,
        Indirect = 1 << 4,
        Dynamic = 1 << 5,
        CPUAccessible = 1 << 6,
    };
    ALIMER_BITMASK(BufferUsage);

    enum class FrontFace : uint32_t
    {
        CounterClockwise = 0,
        Clockwise = 1
    };

    enum class CullMode : uint32_t 
    {
        None = 0,
        Front,
        Back
    };

    struct SwapChainHandle
    {
        /// Native window or view handle.
        void* nativeHandle;
        /// Native display, connection or instance handle.
        void* nativeDisplay;
    };

    /// Describes SwapChain
    struct SwapChainDescriptor
    {
        /// Width.
        uint32_t width;
        /// Height.
        uint32_t height;
        /// Vertical sync
        bool vSync;
        /// Preferred depth stencil format.
        PixelFormat preferredDepthStencilFormat;
        /// Preferred sample count
        SampleCount preferredSamples;
    };

    struct GraphicsDeviceDescriptor
    {
        SwapChainDescriptor     swapchain = {};
    };

    struct TextureDescriptor
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t arraySize;
        uint32_t mipLevels;
        SampleCount samples;
        TextureType type;
        PixelFormat format;
        TextureUsage usage;
    };

    class Texture;
    struct FramebufferAttachment
    {
        /// The texture attachment.
        Texture* texture;
        /// The mipmap level of the texture used for rendering to the attachment.
        uint32_t level;
        /// The slice of the texture used for rendering to the attachment.
        uint32_t slice;
    };

    struct FramebufferDescriptor
    {
        FramebufferAttachment colorAttachments[MaxColorAttachments];
        FramebufferAttachment depthStencilAttachment;
    };

    struct ShaderStageDescriptor
    {
        uint64_t codeSize;
        const uint8_t* code;
        const char* source;
        const char* entryPoint;
    };

    struct ShaderDescriptor
    {
        ShaderStageDescriptor stages[static_cast<unsigned>(ShaderStage::Count)];
    };

    struct BufferDescriptor
    {
        uint64_t size;
        BufferUsage usage;
        uint32_t stride;
    };

    struct SamplerDescriptor
    {
        SamplerAddressMode addressModeU = SamplerAddressMode::Repeat;
        SamplerAddressMode addressModeV = SamplerAddressMode::Repeat;
        SamplerAddressMode addressModeW = SamplerAddressMode::Repeat;
        SamplerMinMagFilter magFilter = SamplerMinMagFilter::Nearest;
        SamplerMinMagFilter minFilter = SamplerMinMagFilter::Nearest;
        SamplerMipFilter mipmapFilter = SamplerMipFilter::Nearest;
        float lodMinClamp = 0;
        float lodMaxClamp = 3.402823466e+38F; // FLT_MAX
        uint32_t maxAnisotropy = 1;
        CompareFunction compareFunction = CompareFunction::Never;
        SamplerBorderColor borderColor = SamplerBorderColor::TransparentBlack;
    };

    struct RasterizationStateDescriptor {
        FrontFace       frontFace;
        CullMode        cullMode;

        uint32_t        depthBias;
        float           depthBiasSlopeScale;
        float           depthBiasClamp;
    };

    struct ColorAttachmentAction
    {
        LoadAction  loadAction;
        StoreAction storeAction;
        Color4      clearColor;
    };

    struct DepthStencilAttachmentAction
    {
        LoadAction  depthLoadAction;
        StoreAction depthStoreAction;
        LoadAction  stencilLoadAction;
        StoreAction stencilStoreAction;
        float       clearDepth;
        uint8_t     clearStencil;
    };

    struct RenderPassBeginDescriptor
    {
        ColorAttachmentAction           colors[MaxColorAttachments];
        DepthStencilAttachmentAction    depthStencil;
    };

    struct VertexElement
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

    ALIMER_API uint32_t GetVertexElementSize(VertexFormat format);

    ALIMER_API const char* EnumToString(ResourceUsage usage);
    ALIMER_API const char* EnumToString(VertexElementSemantic semantic);
}

/* Backend types */
#if defined(ALIMER_D3D11)
struct ID3D11Buffer;
#elif defined(ALIMER_D3D12)
struct ID3D12Resource;
#elif defined(ALIMER_VULKAN)

/* Took from vulkan_core.h */
#ifndef VK_NULL_HANDLE
#define VK_NULL_HANDLE 0
#endif

#ifndef VK_DEFINE_HANDLE
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#endif

#if !defined(VK_DEFINE_NON_DISPATCHABLE_HANDLE)
#   if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__) ) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#       define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#   else
#       define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#   endif
#endif

VK_DEFINE_HANDLE(VkDevice)
VK_DEFINE_HANDLE(VkQueue)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSemaphore)
VK_DEFINE_HANDLE(VkCommandBuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFence)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDeviceMemory)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImage)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkEvent)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkQueryPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBufferView)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImageView)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderModule)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineCache)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineLayout)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkRenderPass)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipeline)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSetLayout)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSampler)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSet)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFramebuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkCommandPool)
#elif defined(ALIMER_OPENGL)
#ifndef GL_INVALID_VALUE
#   define GL_INVALID_VALUE			0x0501
#endif 
#endif
