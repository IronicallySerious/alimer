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

#include "agpu.h"

#if defined(ALIMER_D3D11) || defined(ALIMER_D3D12)
#define AGPU_IMPLEMENTATION
#include "agpu_backend.h"
#include "../Debug/Debug.h"

DXGI_FORMAT agpuD3DConvertPixelFormat(AgpuPixelFormat format)
{
    switch (format)
    {
    case AGPU_PIXEL_FORMAT_UNKNOWN:
        return DXGI_FORMAT_UNKNOWN;

    case AGPU_PIXEL_FORMAT_R8_UNORM:
        return DXGI_FORMAT_R8_UNORM;

    case AGPU_PIXEL_FORMAT_R8_SNORM:
        return DXGI_FORMAT_R8_SNORM;

    case AGPU_PIXEL_FORMAT_R16_UNORM:
        return DXGI_FORMAT_R16_UNORM;

    case AGPU_PIXEL_FORMAT_R16_SNORM:
        return DXGI_FORMAT_R16_SNORM;

    case AGPU_PIXEL_FORMAT_RG8_UNORM:
        return DXGI_FORMAT_R8G8_UNORM;

    case AGPU_PIXEL_FORMAT_RG8_SNORM:
        return DXGI_FORMAT_R8G8_SNORM;

    case AGPU_PIXEL_FORMAT_RG16_UNORM:
        return DXGI_FORMAT_R16G16_UNORM;

    case AGPU_PIXEL_FORMAT_RG16_SNORM:
        return DXGI_FORMAT_R16G16_SNORM;

    case AGPU_PIXEL_FORMAT_RGB16_UNORM:
        return DXGI_FORMAT_UNKNOWN;

    case AGPU_PIXEL_FORMAT_RGB16_SNORM:
        return DXGI_FORMAT_UNKNOWN;

    case AGPU_PIXEL_FORMAT_RGBA8_UNORM:
        return DXGI_FORMAT_R8G8B8A8_UNORM;

    case AGPU_PIXEL_FORMAT_RGBA8_UNORM_SRGB:
        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    case AGPU_PIXEL_FORMAT_RGBA8_SNORM:
        return DXGI_FORMAT_R8G8B8A8_SNORM;

    case AGPU_PIXEL_FORMAT_BGRA8_UNORM:
        return DXGI_FORMAT_B8G8R8A8_UNORM;

    case AGPU_PIXEL_FORMAT_BGRA8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    case AGPU_PIXEL_FORMAT_D32_FLOAT:
        return DXGI_FORMAT_D32_FLOAT;

    case AGPU_PIXEL_FORMAT_D16_UNORM:
        return DXGI_FORMAT_D16_UNORM;

    case AGPU_PIXEL_FORMAT_D24_UNORM_S8:
        return DXGI_FORMAT_D24_UNORM_S8_UINT;

    case AGPU_PIXEL_FORMAT_D32_FLOAT_S8:
        return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

    case AGPU_PIXEL_FORMAT_BC1_UNORM:
        return DXGI_FORMAT_BC1_UNORM;

    case AGPU_PIXEL_FORMAT_BC1_UNORM_SRGB:
        return DXGI_FORMAT_BC1_UNORM_SRGB;

    case AGPU_PIXEL_FORMAT_BC2_UNORM:
        return DXGI_FORMAT_BC2_UNORM;

    case AGPU_PIXEL_FORMAT_BC2_UNORM_SRGB:
        return DXGI_FORMAT_BC2_UNORM_SRGB;

    case AGPU_PIXEL_FORMAT_BC3_UNORM:
        return DXGI_FORMAT_BC3_UNORM;

    case AGPU_PIXEL_FORMAT_BC3_UNORM_SRGB:
        return DXGI_FORMAT_BC3_UNORM_SRGB;

    case AGPU_PIXEL_FORMAT_BC4_UNORM:
        return DXGI_FORMAT_BC4_UNORM;

    case AGPU_PIXEL_FORMAT_BC4_SNORM:
        return DXGI_FORMAT_BC4_SNORM;

    case AGPU_PIXEL_FORMAT_BC5_UNORM:
        return DXGI_FORMAT_BC5_UNORM;

    case AGPU_PIXEL_FORMAT_BC5_SNORM:
        return DXGI_FORMAT_BC5_SNORM;

    case AGPU_PIXEL_FORMAT_BC6HS16:
        return DXGI_FORMAT_BC6H_SF16;

    case AGPU_PIXEL_FORMAT_BC6HU16:
        return DXGI_FORMAT_BC6H_UF16;

    case AGPU_PIXEL_FORMAT_BC7_UNORM:
        return DXGI_FORMAT_BC7_UNORM;

    case AGPU_PIXEL_FORMAT_BC7_UNORM_SRGB:
        return DXGI_FORMAT_BC7_UNORM_SRGB;
    default:
        ALIMER_UNREACHABLE();
    }
}

DXGI_FORMAT agpuD3DConvertVertexFormat(AgpuVertexFormat format)
{
    switch (format)
    {
    case AGPU_VERTEX_FORMAT_FLOAT:		
        return DXGI_FORMAT_R32_FLOAT;

    case AGPU_VERTEX_FORMAT_FLOAT2:		
        return DXGI_FORMAT_R32G32_FLOAT;

    case AGPU_VERTEX_FORMAT_FLOAT3:
        return DXGI_FORMAT_R32G32B32_FLOAT;

    case AGPU_VERTEX_FORMAT_FLOAT4:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case AGPU_VERTEX_FORMAT_BYTE4:
        return DXGI_FORMAT_R8G8B8A8_SINT;

    case AGPU_VERTEX_FORMAT_BYTE4N:
        return DXGI_FORMAT_R8G8B8A8_SNORM;

    case AGPU_VERTEX_FORMAT_UBYTE4:
        return DXGI_FORMAT_R8G8B8A8_UINT;

    case AGPU_VERTEX_FORMAT_UBYTE4N:
        return DXGI_FORMAT_R8G8B8A8_UNORM;

    case AGPU_VERTEX_FORMAT_SHORT2:
        return DXGI_FORMAT_R16G16_SINT;

    case AGPU_VERTEX_FORMAT_SHORT2N:
        return DXGI_FORMAT_R16G16_SNORM;

    case AGPU_VERTEX_FORMAT_SHORT4:
        return DXGI_FORMAT_R16G16B16A16_SINT;

    case AGPU_VERTEX_FORMAT_SHORT4N:
        return DXGI_FORMAT_R16G16B16A16_SNORM;

    default:
        ALIMER_UNREACHABLE();
    }
}

D3D_PRIMITIVE_TOPOLOGY agpuD3DConvertPrimitiveTopology(AgpuPrimitiveTopology topology, uint32_t patchCount)
{
    switch (topology)
    {
    case AGPU_PRIMITIVE_TOPOLOGY_POINT_LIST:
        return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

    case AGPU_PRIMITIVE_TOPOLOGY_LINE_LIST:
        return D3D_PRIMITIVE_TOPOLOGY_LINELIST;

    case AGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP:
        return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;

    case AGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    case AGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

    case AGPU_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
        return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;

    case AGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
        return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;

    case AGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;

    case AGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;

    case AGPU_PRIMITIVE_TOPOLOGY_PATCH_LIST:
        return (D3D_PRIMITIVE_TOPOLOGY)(uint32_t(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) + patchCount);

    default:
        ALIMER_UNREACHABLE();
    }
}

#endif /* ALIMER_D3D12 */
