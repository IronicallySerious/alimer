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
#include "Math/Color.h"
#include "Graphics/PixelFormat.h"

#include <string>
#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable : 4201) 
#endif

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
    enum class GpuPreference : uint32_t {
        /// No GPU preference.
        Default = 0,
        LowPower,
        HighPerformance,
    };

    /// Enum describing the Graphics backend.
    enum class GraphicsBackend : uint32_t {
        /// Null backend.
        Null,
        /// Vulkan backend.
        Vulkan,
        /// Direct3D 12 backend.
        Direct3D12,
        /// Direct3D 11 backend.
        Direct3D11,
        /// Count - Default platform backend
        Count
    };

    enum class QueueType : uint32_t {
        Direct,
        Compute,
        Copy
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
        Store,
        MultisampleResolve,
        StoreAndMultisampleResolve
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

    enum class CompareOp : uint32_t
    {
        Never = 0,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
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
        Type3D,
        TypeCube,
    };

    /// Defines texture usage enum.
    enum class TextureUsage : uint32_t
    {
        /// Unknown usage.
        Unknown = 0,
        /// Specifies an option that enables reading or sampling from the texture.
        ShaderRead = 1 << 0,
        /// Specifies an option that enables writing to the texture.
        ShaderWrite = 1 << 1,
        /// Specifies an option that enables using the texture as a color, depth, or stencil render target 
        RenderTarget = 1 << 2,
    };
    ALIMER_BITMASK(TextureUsage);

    /* Sampler */
    enum class SamplerAddressMode : uint32_t
    {
        /// Texture coordinates are clamped between 0.0 and 1.0, inclusive.
        ClampToEdge,
        /// Texture coordinates wrap to the other side of the texture, effectively keeping only the fractional part of the texture coordinate.
        Repeat,
        /// Between -1.0 and 1.0, the texture coordinates are mirrored across the axis. Outside -1.0 and 1.0, the texture coordinates are clamped.
        MirrorClampToEdge,
        /// Between -1.0 and 1.0, the texture coordinates are mirrored across the axis. Outside -1.0 and 1.0, the image is repeated.
        MirrorRepeat,
        /// Out-of-range texture coordinates return the value specified by the sampler's border color.
        ClampToBorder
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
        std::string name;
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
        Indirect = 1 << 4
    };
    ALIMER_BITMASK(BufferUsage);

    enum class FrontFace : uint32_t
    {
        CounterClockwise = 0,
        Clockwise = 1
    };

    enum class CullMode : uint32_t {
        None = 0,
        Front,
        Back
    };

    /// Defines the faces in a cube map texture.
    enum class TextureCubemapFace : uint8_t {
        /// Positive X face 
        PositiveX = 0,
        /// Negative X face
        NegativeX = 1,
        /// Positive Y face
        PositiveY = 2,
        /// Negative Y face
        NegativeY = 3,
        /// Positive Z face
        PositiveZ = 4,
        /// Negative Z face
        NegativeZ = 5,
    };

    enum class GpuVendor : uint32_t
    {
        Unknown = 0,
        NVIDIA = 1,
        AMD = 2,
        INTEL = 3,
        ARM = 4,
        WARP = 5,
        Count = 6
    };

    /// GraphicsDevice information .
    struct GraphicsDeviceInfo
    {
        /// Rendering API backend.
        GraphicsBackend backend;

        /// Rendering API name.
        std::string backendName;

        /// The hardware gpu device vendor name.
        std::string vendorName;


        /// The hardwarw gpu device vendor id.
        uint32_t    vendorId;

        /// The hardwarw gpu device id.
        uint32_t    deviceId;

        /// The hardwarw gpu device name.
        std::string deviceName;
    };

    /// Describes features supported by given instance of GpuDevice.
    struct GraphicsDeviceFeatures
    {
        bool    instancing = false;
        bool    alphaToCoverage = false;
        bool    independentBlend = false;
        bool    computeShader = false;
        bool    geometryShader = false;
        bool    tessellationShader = false;
        bool    sampleRateShading = false;
        bool    dualSrcBlend = false;
        bool    logicOp = false;
        bool    multiViewport = false;
        bool    indexUInt32 = false;
        bool    drawIndirect = false;
        bool    alphaToOne = false;
        bool    fillModeNonSolid = false;
        bool    samplerAnisotropy = false;
        bool    textureCompressionBC = false;
        bool    textureCompressionPVRTC = false;
        bool    textureCompressionETC2 = false;
        bool    textureCompressionATC = false;
        bool    textureCompressionASTC = false;
        bool    pipelineStatisticsQuery = false;
        /// Specifies whether 1D textures are supported.
        bool    texture1D = false;
        /// Specifies whether 3D textures are supported.
        bool    texture3D = false;
        /// Specifies whether 2D array textures are supported.
        bool    texture2DArray = false;
        /// Specifies whether cube array textures are supported.
        bool    textureCubeArray = false;
        /// Specifies whether raytracing is supported.
        bool    raytracing = false;
    };

    struct GraphicsDeviceLimits
    {
        uint32_t        maxTextureDimension1D;
        uint32_t        maxTextureDimension2D;
        uint32_t        maxTextureDimension3D;
        uint32_t        maxTextureDimensionCube;
        uint32_t        maxTextureArrayLayers;
        uint32_t        maxColorAttachments;
        uint32_t        maxUniformBufferSize;
        uint64_t        minUniformBufferOffsetAlignment;
        uint32_t        maxStorageBufferSize;
        uint64_t        minStorageBufferOffsetAlignment;
        uint32_t        maxSamplerAnisotropy;
        uint32_t        maxViewports;
        uint32_t        maxViewportDimensions[2];
        uint32_t        maxPatchVertices;
        float           pointSizeRange[2];
        float           lineWidthRange[2];
        uint32_t        maxComputeSharedMemorySize;
        uint32_t        maxComputeWorkGroupCount[3];
        uint32_t        maxComputeWorkGroupInvocations;
        uint32_t        maxComputeWorkGroupSize[3];
    };

    /// Describes caps of graphics device.
    struct GraphicsDeviceCapabilities
    {
        /// Specifies all supported hardware features.
        GraphicsDeviceFeatures          features;

        /// Specifies all rendering limitations.
        GraphicsDeviceLimits            limits;
    };


    /// Describes SwapChain
    struct SwapChainDescriptor
    {
        /// Width.
        uint32_t    width = 800;
        /// Height.
        uint32_t    height = 600;

        /// srgb pixel format
        bool        srgb = true;
        bool        depthStencil = true;
        /// Vertical sync
        bool        vsync = true;
        /// Preferred sample count
        SampleCount samples = SampleCount::Count1;
        /// Native window handle (HWND, ANativeWindow, NSWindow).
        uint64_t    nativeHandle;
        /// Native window display (HMODULE, X11Display, WaylandDisplay).
        uint64_t    nativeDisplay;
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
        Texture* texture = nullptr;
        /// The mipmap level of the texture used for rendering to the attachment.
        uint32_t level = 0;
        union {
            /// Cubemap face
            TextureCubemapFace face = TextureCubemapFace::PositiveX;
            /// The slice of the texture used for rendering to the attachment.
            uint32_t slice;
            /// The 3D texture layer.
            uint32_t layer;
        };
    };

    struct FramebufferDescriptor
    {
        FramebufferAttachment colorAttachments[MaxColorAttachments];
        FramebufferAttachment depthStencilAttachment;
    };

    struct ShaderStageDescriptor
    {
        uint64_t codeSize;
        uint8_t* code;
        const char* source;
        const char* entryPoint;
    };

    struct ShaderDescriptor
    {
        ShaderStageDescriptor stages[static_cast<unsigned>(ShaderStage::Count)];
    };

    struct BufferDescriptor
    {
        BufferUsage     usage;
        ResourceUsage   resourceUsage;
        uint64_t        size;
        uint32_t        stride;
    };

    struct SamplerDescriptor
    {
        SamplerAddressMode  addressModeU = SamplerAddressMode::Repeat;
        SamplerAddressMode  addressModeV = SamplerAddressMode::Repeat;
        SamplerAddressMode  addressModeW = SamplerAddressMode::Repeat;
        SamplerMinMagFilter magFilter = SamplerMinMagFilter::Nearest;
        SamplerMinMagFilter minFilter = SamplerMinMagFilter::Nearest;
        SamplerMipFilter    mipmapFilter = SamplerMipFilter::Nearest;
        uint32_t            maxAnisotropy = 1;
        CompareOp           compareOp = CompareOp::Never;
        SamplerBorderColor  borderColor = SamplerBorderColor::TransparentBlack;
        float               lodMinClamp = 0;
        float               lodMaxClamp = 3.402823466e+38F; // FLT_MAX
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
        ColorAttachmentAction           colorAttachments[MaxColorAttachments];
        DepthStencilAttachmentAction    depthStencilAttachment;
    };

    struct RenderPassColorAttachmentDescriptor {
        /// The texture attachment.
        Texture* texture = nullptr;
        /// The mipmap level of the texture used for rendering to the attachment.
        uint32_t level = 0;
        union {
            /// Cubemap face.
            TextureCubemapFace face = TextureCubemapFace::PositiveX;
            /// The slice of the texture used for rendering to the attachment.
            uint32_t slice;
            /// The depth plane of a 3D texture used for rendering to the attachment.
            uint32_t depthPlane;
        };

        /// The destination texture used when multisampled texture data is resolved into single sample values.
        Texture* resolveTexture = nullptr;

        /// The mipmap level of the texture used for the multisample resolve action.
        uint32_t resolveLevel = 0;
        union {
            /// The cubemap face of the texture used for the multisample resolve action.
            TextureCubemapFace resolveFace = TextureCubemapFace::PositiveX;
            /// The slice of the texture used for the multisample resolve action.
            uint32_t resolveSlice;
            /// The depth plane of the 3D texture used for the multisample resolve action.
            uint32_t resolveDepthPlane;
        };

        LoadAction  loadAction = LoadAction::Clear;
        StoreAction storeAction = StoreAction::Store;
        Color4      clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    };

    struct RenderPassDepthStencilAttachmentDescriptor {
        /// The texture attachment.
        Texture* texture = nullptr;
        /// The mipmap level of the texture used for rendering to the attachment.
        uint32_t level = 0;
        union {
            /// Cubemap face.
            TextureCubemapFace face = TextureCubemapFace::PositiveX;
            /// The slice of the texture used for rendering to the attachment.
            uint32_t slice;
            /// The depth plane of a 3D texture used for rendering to the attachment.
            uint32_t depthPlane;
        };

        LoadAction  depthLoadAction = LoadAction::Clear;
        StoreAction depthStoreAction = StoreAction::Store;
        float       clearDepth = 1.0f;

        LoadAction  stencilLoadAction = LoadAction::DontCare;
        StoreAction stencilStoreAction = StoreAction::DontCare;
        uint8_t     clearStencil = 0;
    };

    struct RenderPassDescriptor {
        uint32_t colorAttachmentCount = 0;
        RenderPassColorAttachmentDescriptor colorAttachments[MaxColorAttachments];
        const RenderPassDepthStencilAttachmentDescriptor* depthStencilAttachment;

        /// The width, in pixels, to constrain the render target to.
        uint32_t renderTargetWidth = 0;

        /// The height, in pixels, to constrain the render target to.
        uint32_t renderTargetHeight = 0;

        /// The number of active layers that all attachments must have for layered rendering.
        uint32_t renderTargetArraySize = 0;
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

    struct VertexBufferLayoutDescriptor {
        uint32_t                        stride;
        VertexInputRate                 inputRate;
    };

    struct VertexAttributeDescriptor {
        VertexFormat                    format;
        uint32_t                        offset;
        uint32_t                        bufferIndex;
    };

    struct VertexDescriptor {
        VertexBufferLayoutDescriptor    layouts[MaxVertexBufferBindings];
        VertexAttributeDescriptor       attributes[MaxVertexAttributes];
    };

    class Shader;
    struct RenderPipelineDescriptor {
        Shader*                         vertexShader;
        Shader*                         fragmentShader;
        RasterizationStateDescriptor    rasterizationState;
        //BlendStateDescriptor            blendStates;
        //DepthStencilStateDescriptor     depthStencilState;
        VertexDescriptor                vertexDescriptor;
        PrimitiveTopology               primitiveTopology = PrimitiveTopology::TriangleList;
    };

    struct ComputePipelineDescriptor {
    };

    ALIMER_API uint32_t GetVertexElementSize(VertexFormat format);

    ALIMER_API const char* EnumToString(ResourceUsage usage);
    ALIMER_API const char* EnumToString(VertexElementSemantic semantic);
}

#ifdef _MSC_VER
#   pragma warning(pop)
#endif

/* Backend declaration */
#if defined(ALIMER_VULKAN)

#ifndef VK_NULL_HANDLE
#   define VK_NULL_HANDLE 0
#endif

#ifndef VK_DEFINE_HANDLE
#   define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#endif


#if !defined(VK_DEFINE_NON_DISPATCHABLE_HANDLE)
#   if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__) ) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#       define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#   else
#       define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#   endif
#endif

VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBuffer)

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)

#elif defined(ALIMER_D3D12)
#elif defined(ALIMER_D3D11)
#endif
