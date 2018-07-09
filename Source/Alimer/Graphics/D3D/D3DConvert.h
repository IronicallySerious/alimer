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

#include "D3DPrerequisites.h"
#include "../Types.h"
#include "../PixelFormat.h"
#include "../GpuBuffer.h"

namespace Alimer
{
    namespace d3d
    {
        static inline DXGI_FORMAT Convert(PixelFormat format)
        {
            switch (format)
            {
                case PixelFormat::R8UNorm:              return DXGI_FORMAT_R8_UNORM;
                case PixelFormat::RG8UNorm:             return DXGI_FORMAT_R8G8_UNORM;
                case PixelFormat::RGBA8UNorm:			return DXGI_FORMAT_R8G8B8A8_UNORM;
                case PixelFormat::BGRA8UNorm:			return DXGI_FORMAT_B8G8R8A8_UNORM;
                    //case PixelFormat::BGRA8UNorm_SRGB:		return VK_FORMAT_B8G8R8A8_SRGB;
                    //case PixelFormat::RGBA8UNorm_SRGB:		return VK_FORMAT_R8G8B8A8_SRGB;
                    //case PixelFormat::Depth16UNorm:			return VK_FORMAT_D16_UNORM;
                    //case PixelFormat::Depth16UNormStencil8:	return VK_FORMAT_D16_UNORM_S8_UINT;
                    //case PixelFormat::Depth32Float:			return VK_FORMAT_D32_SFLOAT;
                    //case PixelFormat::Stencil8:				return VK_FORMAT_S8_UINT;
                    //case PixelFormat::Depth24UNormStencil8:	return VK_FORMAT_D24_UNORM_S8_UINT;
                    //case PixelFormat::Depth32FloatStencil8: return VK_FORMAT_D32_SFLOAT_S8_UINT;
                    //case PixelFormat::BC1:					return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
                    //case PixelFormat::BC1_SRGB:				return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
                    //case PixelFormat::BC2:					return VK_FORMAT_BC2_UNORM_BLOCK;
                    //case PixelFormat::BC2_SRGB:				return VK_FORMAT_BC2_SRGB_BLOCK;
                    //case PixelFormat::BC3:					return VK_FORMAT_BC3_UNORM_BLOCK;
                    //case PixelFormat::BC3_SRGB:				return VK_FORMAT_BC3_SRGB_BLOCK;
                default:
                    ALIMER_UNREACHABLE();
                    return DXGI_FORMAT_UNKNOWN;
            }
        }

        static inline PixelFormat Convert(DXGI_FORMAT format)
        {
            switch (format)
            {
                case DXGI_FORMAT_R8_UNORM:            return PixelFormat::R8UNorm;
                case DXGI_FORMAT_R8G8_UNORM:          return PixelFormat::RG8UNorm;
                case DXGI_FORMAT_R8G8B8A8_UNORM:		return PixelFormat::RGBA8UNorm;
                case DXGI_FORMAT_B8G8R8A8_UNORM:		return PixelFormat::BGRA8UNorm;
                default:
                    return PixelFormat::Undefined;
            }
        }

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

        static inline DXGI_FORMAT Convert(IndexType type)
        {
            switch (type)
            {
            case IndexType::UInt16: return DXGI_FORMAT_R16_UINT;
            case IndexType::UInt32: return DXGI_FORMAT_R32_UINT;
            default:
                ALIMER_UNREACHABLE();
            }
        }

        static inline D3D_PRIMITIVE_TOPOLOGY Convert(PrimitiveTopology topology)
        {
            switch (topology)
            {
            case PrimitiveTopology::Points:
                return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case PrimitiveTopology::Lines:
                return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            case PrimitiveTopology::LineStrip:
                return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case PrimitiveTopology::Triangles:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case PrimitiveTopology::TriangleStrip:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

            default:
                return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        }
    }
}
