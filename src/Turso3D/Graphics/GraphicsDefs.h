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

#include "Turso3DConfig.h"
#include "../Base/String.h"
#include "../Math/IntRect.h"

namespace Turso3D
{
    /// Clear rendertarget color.
    static const unsigned CLEAR_COLOR = 1;
    /// Clear rendertarget depth.
    static const unsigned CLEAR_DEPTH = 2;
    /// Clear rendertarget stencil.
    static const unsigned CLEAR_STENCIL = 4;
    /// Clear color+depth+stencil.
    static const unsigned CLEAR_ALL = 7;

    /// Maximum simultaneous vertex buffers.
    static constexpr uint32_t MAX_VERTEX_STREAMS = 4u;
    /// Maximum simultaneous constant buffers.
    static constexpr uint32_t MAX_CONSTANT_BUFFERS = 15u;
    /// Maximum number of textures in use at once.
    static constexpr uint32_t MAX_TEXTURE_UNITS = 16u;
    /// Maximum number of textures reserved for materials, starting from 0.
    static constexpr uint32_t MAX_MATERIAL_TEXTURE_UNITS = 8u;
    /// Maximum number of color rendertargets in use at once.
    static constexpr uint32_t MAX_RENDERTARGETS = 4u;
    /// Number of cube map faces.
    static constexpr uint32_t MAX_CUBE_FACES = 6u;

    enum class ColorWriteMask : uint8_t
    {
        /// Disable color write.
        None = 0x0,

        /// The red color channel is enabled.
        Red = 0x1,

        /// The green color channel is enabled.
        Green = 0x2,

        /// The blue color channel is enabled.
        Blue = 0x4,

        /// The alpha color channel is enabled.
        Alpha = 0x8,

        /// All color channels are enabled.
        All = 0xf,
    };

    DEFINE_ENUM_CLASS_FLAG_OPERATOR(ColorWriteMask, &);
    DEFINE_ENUM_CLASS_FLAG_OPERATOR(ColorWriteMask, | );

    enum class BufferType : unsigned
    {
        Vertex = 0,
        Index,
        Uniform
    };

    enum class IndexType : unsigned
    {
        UInt16,
        UInt32,
    };

    /// Shader stages.
    enum ShaderStage
    {
        SHADER_VS = 0,
        SHADER_PS,
        MAX_SHADER_STAGES
    };

    /// Element types for constant buffers and vertex elements.
    enum ElementType
    {
        ELEM_INT = 0,
        ELEM_FLOAT,
        ELEM_VECTOR2,
        ELEM_VECTOR3,
        ELEM_VECTOR4,
        ELEM_UBYTE4,
        ELEM_MATRIX3X4,
        ELEM_MATRIX4,
        MAX_ELEMENT_TYPES
    };

    /// Element semantics for vertex elements.
    enum ElementSemantic
    {
        SEM_POSITION = 0,
        SEM_NORMAL,
        SEM_BINORMAL,
        SEM_TANGENT,
        SEM_TEXCOORD,
        SEM_COLOR,
        SEM_BLENDWEIGHT,
        SEM_BLENDINDICES,
        MAX_ELEMENT_SEMANTICS
    };

    /// Primitive types.
    enum PrimitiveType
    {
        POINT_LIST = 1,
        LINE_LIST,
        LINE_STRIP,
        TRIANGLE_LIST,
        TRIANGLE_STRIP,
        MAX_PRIMITIVE_TYPES
    };

    /// Blend factors.
    enum BlendFactor
    {
        BLEND_ZERO = 1,
        BLEND_ONE,
        BLEND_SRC_COLOR,
        BLEND_INV_SRC_COLOR,
        BLEND_SRC_ALPHA,
        BLEND_INV_SRC_ALPHA,
        BLEND_DEST_ALPHA,
        BLEND_INV_DEST_ALPHA,
        BLEND_DEST_COLOR,
        BLEND_INV_DEST_COLOR,
        BLEND_SRC_ALPHA_SAT,
        MAX_BLEND_FACTORS
    };

    /// Blend operations.
    enum BlendOp
    {
        BLEND_OP_ADD = 1,
        BLEND_OP_SUBTRACT,
        BLEND_OP_REV_SUBTRACT,
        BLEND_OP_MIN,
        BLEND_OP_MAX,
        MAX_BLEND_OPS
    };

    /// Predefined blend modes.
    enum BlendMode
    {
        BLEND_MODE_REPLACE = 0,
        BLEND_MODE_ADD,
        BLEND_MODE_MULTIPLY,
        BLEND_MODE_ALPHA,
        BLEND_MODE_ADDALPHA,
        BLEND_MODE_PREMULALPHA,
        BLEND_MODE_INVDESTALPHA,
        BLEND_MODE_SUBTRACT,
        BLEND_MODE_SUBTRACTALPHA,
        MAX_BLEND_MODES
    };

    /// Fill modes.
    enum FillMode
    {
        FILL_WIREFRAME = 2,
        FILL_SOLID = 3,
        MAX_FILL_MODES
    };

    /// Triangle culling modes.
    enum CullMode
    {
        CULL_NONE = 1,
        CULL_FRONT,
        CULL_BACK,
        MAX_CULL_MODES
    };

    /// Specifies comparison options that should be performed on a depth texture.
    enum CompareFunction : uint32_t
    {
        /// A new value never passes the comparison test.
        Never = 1,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        /// A new value always passes the comparison test.
        Always
    };

    /// Identifies the stencil operations that can be performed during depth-stencil testing.
    enum class StencilOperation : uint32_t
    {
        /// Keep the current stencil value.
        Keep = 1,
        /// Set the stencil data to 0.
        Zero,
        /// Replace the stencil value with the stencil reference value, which is set by the SetStencilReference
        Replace,
        /// Increment the stencil value by 1, and clamp the result.
        IncrementClamp,
        /// Decrement the stencil value by 1, and clamp the result.
        DecrementClamp,
        /// Invert the stencil data.
        Invert,
        /// Increment the stencil value by 1, and wrap the result if necessary.
        IncrementWrap,
        /// Decrement the stencil value by 1, and wrap the result if necessary.
        DecrementWrap
    };

    /// Texture types.
    enum class TextureType : uint32_t
    {
        Type1D = 0,
        Type2D,
        Type3D,
        TypeCube,
    };

    /// Resource usage modes. Rendertarget usage can only be used with textures.
    enum class ResourceUsage : uint32_t
    {
        Default = 0,
        Immutable,
        Dynamic,
        RenderTarget
    };

    /// Texture filtering modes.
    enum TextureFilterMode
    {
        FILTER_POINT = 0,
        FILTER_BILINEAR,
        FILTER_TRILINEAR,
        FILTER_ANISOTROPIC,
        COMPARE_POINT,
        COMPARE_BILINEAR,
        COMPARE_TRILINEAR,
        COMPARE_ANISOTROPIC
    };

    /// Texture addressing modes.
    enum TextureAddressMode
    {
        ADDRESS_WRAP = 1,
        ADDRESS_MIRROR,
        ADDRESS_CLAMP,
        ADDRESS_BORDER,
        ADDRESS_MIRROR_ONCE
    };

    /// Description of an element in a vertex declaration.
    struct TURSO3D_API VertexElement
    {
        /// Default-construct.
        VertexElement() :
            type(ELEM_VECTOR3),
            semantic(SEM_POSITION),
            index(0),
            perInstance(false),
            offset(0)
        {
        }

        /// Construct with type, semantic, index and whether is per-instance data.
        VertexElement(ElementType type_, ElementSemantic semantic_, unsigned char index_ = 0, bool perInstance_ = false) :
            type(type_),
            semantic(semantic_),
            index(index_),
            perInstance(perInstance_),
            offset(0)
        {
        }

        /// Data type of element.
        ElementType type;
        /// Semantic of element.
        ElementSemantic semantic;
        /// Semantic index of element, for example multi-texcoords.
        unsigned char index;
        /// Per-instance flag.
        bool perInstance;
        /// Offset of element from vertex start. Filled by VertexBuffer.
        size_t offset;
    };

    /// Description of a shader constant.
    struct TURSO3D_API Constant
    {
        /// Construct empty.
        Constant() = default;

        /// Construct with type, name and optional number of elements.
        Constant(ElementType type_, const String& name_, uint32_t numElements_ = 1)
            : type(type_)
            , name(name_)
            , numElements(numElements_)
        {
        }

        /// Construct with type, name and optional number of elements.
        Constant(ElementType type_, const char* name_, uint32_t numElements_ = 1)
            : type(type_)
            , name(name_)
            , numElements(numElements_)
        {
        }

        /// Data type of constant.
        ElementType type;
        /// Name of constant.
        String name;
        /// Number of elements. Default 1.
        uint32_t numElements = 1;
        /// Element size. Filled by ConstantBuffer.
        uint32_t elementSize;
        /// Offset from the beginning of the buffer. Filled by ConstantBuffer.
        uint32_t offset = 0;
    };

    /// Description of a blend mode.
    struct TURSO3D_API BlendModeDesc
    {
        /// Default-construct.
        BlendModeDesc()
        {
            Reset();
        }

        /// Construct with parameters.
        BlendModeDesc(bool blendEnable_, BlendFactor srcBlend_, BlendFactor destBlend_, BlendOp blendOp_, BlendFactor srcBlendAlpha_, BlendFactor destBlendAlpha_, BlendOp blendOpAlpha_) :
            blendEnable(blendEnable_),
            srcBlend(srcBlend_),
            destBlend(destBlend_),
            blendOp(blendOp_),
            srcBlendAlpha(srcBlendAlpha_),
            destBlendAlpha(destBlendAlpha_),
            blendOpAlpha(blendOpAlpha_)
        {
        }

        /// Reset to defaults.
        void Reset()
        {
            blendEnable = false;
            srcBlend = BLEND_ONE;
            destBlend = BLEND_ONE;
            blendOp = BLEND_OP_ADD;
            srcBlendAlpha = BLEND_ONE;
            destBlendAlpha = BLEND_ONE;
            blendOpAlpha = BLEND_OP_ADD;
        }

        /// Test for equality with another blend mode description.
        bool operator == (const BlendModeDesc& rhs) const { return blendEnable == rhs.blendEnable && srcBlend == rhs.srcBlend && destBlend == rhs.destBlend && blendOp == rhs.blendOp && srcBlendAlpha == rhs.srcBlendAlpha && destBlendAlpha == rhs.destBlendAlpha && blendOpAlpha == rhs.blendOpAlpha; }
        /// Test for inequality with another blend mode description.
        bool operator != (const BlendModeDesc& rhs) const { return !(*this == rhs); }

        /// Blend enable flag.
        bool blendEnable;
        /// Source color blend factor.
        BlendFactor srcBlend;
        /// Destination color blend factor.
        BlendFactor destBlend;
        /// Color blend operation.
        BlendOp blendOp;
        /// Source alpha blend factor.
        BlendFactor srcBlendAlpha;
        /// Destination alpha blend factor.
        BlendFactor destBlendAlpha;
        /// Alpha blend operation.
        BlendOp blendOpAlpha;
    };

    /// Description of a stencil test.
    struct TURSO3D_API StencilTestDesc
    {
        /// Default-construct.
        StencilTestDesc()
        {
            Reset();
        }

        /// Reset to defaults.
        void Reset()
        {
            stencilReadMask = 0xff;
            stencilWriteMask = 0xff;
            frontFunc = CompareFunction::Always;
            frontFail = StencilOperation::Keep;
            frontDepthFail = StencilOperation::Keep;
            frontPass = StencilOperation::Keep;
            backFunc = CompareFunction::Always;
            backFail = StencilOperation::Keep;
            backDepthFail = StencilOperation::Keep;
            backPass = StencilOperation::Keep;
        }

        /// Stencil read bit mask.
        uint8_t stencilReadMask;
        /// Stencil write bit mask.
        uint8_t stencilWriteMask;
        /// Stencil front face compare function.
        CompareFunction frontFunc;
        /// Operation for front face stencil test fail.
        StencilOperation frontFail;
        /// Operation for front face depth test fail.
        StencilOperation frontDepthFail;
        /// Operation for front face pass.
        StencilOperation frontPass;
        /// Stencil back face compare function.
        CompareFunction backFunc;
        /// Operation for back face stencil test fail.
        StencilOperation backFail;
        /// Operation for back face depth test fail.
        StencilOperation backDepthFail;
        /// Operation for back face pass.
        StencilOperation backPass;
    };

    /// Collection of render state.
    struct RenderState
    {
        /// Default-construct.
        RenderState()
        {
            Reset();
        }

        /// Reset to defaults.
        void Reset()
        {
            depthFunc = CompareFunction::LessEqual;
            depthWrite = true;
            depthClip = true;
            depthBias = 0;
            slopeScaledDepthBias = 0.0f;
            colorWriteMask = ColorWriteMask::All;
            alphaToCoverage = false;
            blendMode.Reset();
            cullMode = CULL_BACK;
            fillMode = FILL_SOLID;
            scissorEnable = false;
            scissorRect = IntRect::ZERO;
            stencilEnable = false;
            stencilRef = 0;
            stencilTest.Reset();
        }

        /// Depth test function.
        CompareFunction depthFunc;
        /// Depth write enable.
        bool depthWrite;
        /// Depth clipping enable.
        bool depthClip;
        /// Constant depth bias.
        int depthBias;
        /// Slope-scaled depth bias.
        float slopeScaledDepthBias;
        /// Rendertarget color channel write mask.
        ColorWriteMask colorWriteMask;
        /// Alpha-to-coverage enable.
        bool alphaToCoverage;
        /// Blend mode parameters.
        BlendModeDesc blendMode;
        /// Polygon culling mode.
        CullMode cullMode;
        /// Polygon fill mode.
        FillMode fillMode;
        /// Scissor test enable.
        bool scissorEnable;
        /// Scissor rectangle as pixels from rendertarget top left corner.
        IntRect scissorRect;
        /// Stencil test enable.
        bool stencilEnable;
        /// Stencil reference value.
        unsigned char stencilRef;
        /// Stencil test parameters.
        StencilTestDesc stencilTest;
    };

    /// Vertex element sizes by element type.
    extern TURSO3D_API const uint32_t elementSizes[];
    /// Resource usage names.
    extern TURSO3D_API const char* resourceUsageNames[];
    /// Element type names.
    extern TURSO3D_API const char* elementTypeNames[];
    /// Vertex element semantic names.
    extern TURSO3D_API const char* elementSemanticNames[];
    /// Blend factor names.
    extern TURSO3D_API const char* blendFactorNames[];
    /// Blend operation names.
    extern TURSO3D_API const char* blendOpNames[];
    /// Predefined blend mode names.
    extern TURSO3D_API const char* blendModeNames[];
    /// Fill mode names.
    extern TURSO3D_API const char* fillModeNames[];
    /// Culling mode names.
    extern TURSO3D_API const char* cullModeNames[];
    /// Compare function names.
    extern TURSO3D_API const char* compareFuncNames[];
    /// Stencil operation names.
    extern TURSO3D_API const char* stencilOpNames[];
    /// Predefined blend modes.
    extern TURSO3D_API const BlendModeDesc blendModes[];
}
