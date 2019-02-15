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

#include "D3DPrerequisites.h"
#include "../Types.h"
#include "../PixelFormat.h"

namespace alimer
{
    struct DxgiFormatDesc
    {
        PixelFormat format;
        DXGI_FORMAT dxgiFormat;
    };

    extern const DxgiFormatDesc s_DxgiFormatDesc[];

    inline DXGI_FORMAT GetDxgiFormat(PixelFormat format)
    {
        assert(s_DxgiFormatDesc[(uint32_t)format].format == format);
        return s_DxgiFormatDesc[(uint32_t)format].dxgiFormat;
    }

    inline DXGI_FORMAT GetDxgiTypelessDepthFormat(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::Depth16UNorm:
            return DXGI_FORMAT_R16_TYPELESS;

        case PixelFormat::Depth32Float:
            return DXGI_FORMAT_R32_TYPELESS;

        case PixelFormat::Depth24UNormStencil8:
            return DXGI_FORMAT_R24G8_TYPELESS;

        case PixelFormat::Depth32FloatStencil8:
            return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        case PixelFormat::Stencil8:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        
        default:
            assert(IsDepthStencilFormat(format) == false);
            assert(s_DxgiFormatDesc[(uint32_t)format].format == format);
            return s_DxgiFormatDesc[(uint32_t)format].dxgiFormat;
        }
    }

    inline PixelFormat GetPixelFormatDxgiFormat(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_R8_UNORM:          
            return PixelFormat::R8UNorm;

        case DXGI_FORMAT_R8G8_UNORM:        
            return PixelFormat::RG8UNorm;

        case DXGI_FORMAT_R8G8B8A8_UNORM:    
            return PixelFormat::RGBA8UNorm;

        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return PixelFormat::RGBA8UNormSrgb;

        case DXGI_FORMAT_B8G8R8A8_UNORM:    
            return PixelFormat::BGRA8UNorm;

        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return PixelFormat::BGRA8UNormSrgb;
        default:
            return PixelFormat::Undefined;
        }
    }

    namespace d3d
    {
        static inline DXGI_FORMAT Convert(VertexFormat format)
        {
            switch (format)
            {
            case VertexFormat::Float:		return DXGI_FORMAT_R32_FLOAT;
            case VertexFormat::Float2:		return DXGI_FORMAT_R32G32_FLOAT;
            case VertexFormat::Float3:		return DXGI_FORMAT_R32G32B32_FLOAT;
            case VertexFormat::Float4:		return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case VertexFormat::Byte4:       return DXGI_FORMAT_R8G8B8A8_SINT;
            case VertexFormat::Byte4N:      return DXGI_FORMAT_R8G8B8A8_SNORM;
            case VertexFormat::UByte4:      return DXGI_FORMAT_R8G8B8A8_UINT;
            case VertexFormat::UByte4N:     return DXGI_FORMAT_R8G8B8A8_UNORM;
            case VertexFormat::Short2:      return DXGI_FORMAT_R16G16_SINT;
            case VertexFormat::Short2N:     return DXGI_FORMAT_R16G16_SNORM;
            case VertexFormat::Short4:      return DXGI_FORMAT_R16G16B16A16_SINT;
            case VertexFormat::Short4N:     return DXGI_FORMAT_R16G16B16A16_SNORM;
            default:
                ALIMER_UNREACHABLE();
                return DXGI_FORMAT_UNKNOWN;
            }
        }

        static inline DXGI_FORMAT Convert(IndexType indexType)
        {
            switch (indexType)
            {
            case IndexType::UInt16:		return DXGI_FORMAT_R16_UINT;
            case IndexType::UInt32:		return DXGI_FORMAT_R32_UINT;
           
            default:
                ALIMER_UNREACHABLE();
                return DXGI_FORMAT_UNKNOWN;
            }
        }

        static inline D3D11_INPUT_CLASSIFICATION Convert(VertexInputRate rate)
        {
            switch (rate)
            {
            case VertexInputRate::Vertex:		return D3D11_INPUT_PER_VERTEX_DATA;
            case VertexInputRate::Instance:		return D3D11_INPUT_PER_INSTANCE_DATA;
            default:
                ALIMER_UNREACHABLE();
            }
        }

        static inline D3D_PRIMITIVE_TOPOLOGY Convert(PrimitiveTopology topology, uint32_t patchCount)
        {
            switch (topology)
            {
            case PrimitiveTopology::PointList:
                return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

            case PrimitiveTopology::LineList:
                return D3D_PRIMITIVE_TOPOLOGY_LINELIST;

            case PrimitiveTopology::LineStrip:
                return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;

            case PrimitiveTopology::TriangleList:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

            case PrimitiveTopology::TriangleStrip:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

            case PrimitiveTopology::LineListWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;

            case PrimitiveTopology::LineStripWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;

            case PrimitiveTopology::TriangleListWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;

            case PrimitiveTopology::TriangleStripWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;

            case PrimitiveTopology::PatchList:
                return (D3D_PRIMITIVE_TOPOLOGY)(uint32_t(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) + patchCount);

            default:
                return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        }

        static inline const char* Convert(VertexElementSemantic semantic, uint32_t& semanticIndex)
        {
            semanticIndex = 0;
            switch (semantic)
            {
            case VertexElementSemantic::Position:       return "POSITION";
            case VertexElementSemantic::Normal:         return "NORMAL";
            case VertexElementSemantic::Binormal:       return "BINORMAL";
            case VertexElementSemantic::Tangent:        return "TANGENT";
            case VertexElementSemantic::BlendWeight:    return "BLENDWEIGHT";
            case VertexElementSemantic::BlendIndices:   return "BLENDINDICES";
            case VertexElementSemantic::Color0:
            case VertexElementSemantic::Color1:
            case VertexElementSemantic::Color2:
            case VertexElementSemantic::Color3:
            {
                semanticIndex = static_cast<uint32_t>(semantic) - static_cast<uint32_t>(VertexElementSemantic::Color0);
                return "COLOR";
            }
            case VertexElementSemantic::Texcoord0:
            case VertexElementSemantic::Texcoord1:
            case VertexElementSemantic::Texcoord2:
            case VertexElementSemantic::Texcoord3:
            case VertexElementSemantic::Texcoord4:
            case VertexElementSemantic::Texcoord5:
            case VertexElementSemantic::Texcoord6:
            case VertexElementSemantic::Texcoord7:
            {
                semanticIndex = static_cast<uint32_t>(semantic) - static_cast<uint32_t>(VertexElementSemantic::Texcoord0);
                return "TEXCOORD";
            }
            default: return nullptr;
            }
        }
    }
}
