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

#include "D3DConvert.h"

namespace Alimer
{
    const DxgiFormatDesc s_DxgiFormatDesc[] =
    {
        { PixelFormat::Unknown,             DXGI_FORMAT_UNKNOWN},
        { PixelFormat::R8UNorm,             DXGI_FORMAT_R8_UNORM},
        { PixelFormat::R8SNorm,             DXGI_FORMAT_R8_SNORM},
        { PixelFormat::R16UNorm,            DXGI_FORMAT_R16_UNORM},
        { PixelFormat::R16SNorm,            DXGI_FORMAT_R16_SNORM},
        { PixelFormat::RG8UNorm,            DXGI_FORMAT_R8G8_UNORM},
        { PixelFormat::RG8SNorm,            DXGI_FORMAT_R8G8_SNORM},
        { PixelFormat::RG16UNorm,           DXGI_FORMAT_R16G16_UNORM},
        { PixelFormat::RG16SNorm,           DXGI_FORMAT_R16G16_SNORM},
        { PixelFormat::RGB16UNorm,          DXGI_FORMAT_UNKNOWN},
        { PixelFormat::RGB16SNorm,          DXGI_FORMAT_UNKNOWN},
        //{PixelFormat::R24UNormX8,         DXGI_FORMAT_R24_UNORM_X8_TYPELESS},
        //{PixelFormat::RGB5A1UNorm,        DXGI_FORMAT_B5G5R5A1_UNORM},

        { PixelFormat::RGBA8UNorm,          DXGI_FORMAT_R8G8B8A8_UNORM},
        { PixelFormat::RGBA8UNormSrgb,      DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
        { PixelFormat::RGBA8SNorm,          DXGI_FORMAT_R8G8B8A8_SNORM},

        { PixelFormat::BGRA8UNorm,          DXGI_FORMAT_B8G8R8A8_UNORM},
        { PixelFormat::BGRA8UNormSrgb,      DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
        //{PixelFormat::RGB10A2UNorm,       DXGI_FORMAT_R10G10B10A2_UNORM},
        //{PixelFormat::RGB10A2UInt,        DXGI_FORMAT_R10G10B10A2_UINT},
        //{PixelFormat::RGBA16UNorm,        DXGI_FORMAT_R16G16B16A16_UNORM},
        //{PixelFormat::R16Float,           DXGI_FORMAT_R16_FLOAT},
        //{PixelFormat::RG16Float,          DXGI_FORMAT_R16G16_FLOAT},
        //{PixelFormat::RGB16Float,         DXGI_FORMAT_UNKNOWN},
        //{PixelFormat::RGBA16Float,        DXGI_FORMAT_R16G16B16A16_FLOAT},
        //{PixelFormat::R32Float,           DXGI_FORMAT_R32_FLOAT},
        //{PixelFormat::R32FloatX32,        DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS},
        //{PixelFormat::RG32Float,          DXGI_FORMAT_R32G32_FLOAT},
        //{PixelFormat::RGB32Float,         DXGI_FORMAT_R32G32B32_FLOAT},
        //{PixelFormat::RGBA32Float,        DXGI_FORMAT_R32G32B32A32_FLOAT},
        //{PixelFormat::R11G11B10Float,     DXGI_FORMAT_R11G11B10_FLOAT},
        //{PixelFormat::RGB9E5Float,        DXGI_FORMAT_R9G9B9E5_SHAREDEXP},
        //{PixelFormat::R8Int,              DXGI_FORMAT_R8_SINT},
        //{PixelFormat::R8UInt,             DXGI_FORMAT_R8_UINT},
        //{PixelFormat::R16Int,             DXGI_FORMAT_R16_SINT},
        //{PixelFormat::R16Uint,            DXGI_FORMAT_R16_UINT},
        //{PixelFormat::R32Int,             DXGI_FORMAT_R32_SINT},
        //{PixelFormat::R32Uint,            DXGI_FORMAT_R32_UINT},
        //{PixelFormat::RG8Int,             DXGI_FORMAT_R8G8_SINT},
        //{PixelFormat::RG8Uint,            DXGI_FORMAT_R8G8_UINT},
        //{PixelFormat::RG16Int,            DXGI_FORMAT_R16G16_SINT},
        //{PixelFormat::RG16Uint,           DXGI_FORMAT_R16G16_UINT},
        //{PixelFormat::RG32Int,            DXGI_FORMAT_R32G32_SINT},
        //{PixelFormat::RG32Uint,           DXGI_FORMAT_R32G32_UINT},
        //{PixelFormat::RGB16Int,           DXGI_FORMAT_UNKNOWN},
        //{PixelFormat::RGB16Uint,          DXGI_FORMAT_UNKNOWN},
        //{PixelFormat::RGB32Int,           DXGI_FORMAT_R32G32B32_SINT},
        //{PixelFormat::RGB32Uint,          DXGI_FORMAT_R32G32B32_UINT},
        //{PixelFormat::RGBA8Int,           DXGI_FORMAT_R8G8B8A8_SINT},
        //{PixelFormat::RGBA8Uint,          DXGI_FORMAT_R8G8B8A8_UINT},
        //{PixelFormat::RGBA16Int,          DXGI_FORMAT_R16G16B16A16_SINT},
        //{PixelFormat::RGBA16Uint,         DXGI_FORMAT_R16G16B16A16_UINT},
        //{PixelFormat::RGBA32Int,          DXGI_FORMAT_R32G32B32A32_SINT},
        //{PixelFormat::RGBA32Uint,         DXGI_FORMAT_R32G32B32A32_UINT},

        //{PixelFormat::BGRX8Unorm,         DXGI_FORMAT_B8G8R8X8_UNORM},
        //{PixelFormat::BGRX8UnormSrgb,     DXGI_FORMAT_B8G8R8X8_UNORM_SRGB},
        //{PixelFormat::Alpha8Unorm,        DXGI_FORMAT_A8_UNORM},
        //{PixelFormat::Alpha32Float,       DXGI_FORMAT_UNKNOWN},
        //{PixelFormat::R5G6B5Unorm,        DXGI_FORMAT_B5G6R5_UNORM},

        // Depth-stencil
        { PixelFormat::D32Float,             DXGI_FORMAT_D32_FLOAT},
        { PixelFormat::D16UNorm,             DXGI_FORMAT_D16_UNORM},
        { PixelFormat::D24UNormS8,           DXGI_FORMAT_D24_UNORM_S8_UINT},
        { PixelFormat::D32FloatS8,           DXGI_FORMAT_D32_FLOAT_S8X24_UINT},

        // Compressed formats
        { PixelFormat::BC1UNorm,             DXGI_FORMAT_BC1_UNORM},
        { PixelFormat::BC1UNormSrgb,         DXGI_FORMAT_BC1_UNORM_SRGB},
        { PixelFormat::BC2UNorm,             DXGI_FORMAT_BC2_UNORM},
        { PixelFormat::BC2UNormSrgb,         DXGI_FORMAT_BC2_UNORM_SRGB},
        { PixelFormat::BC3UNorm,             DXGI_FORMAT_BC3_UNORM},
        { PixelFormat::BC3UNormSrgb,         DXGI_FORMAT_BC3_UNORM_SRGB},
        { PixelFormat::BC4UNorm,             DXGI_FORMAT_BC4_UNORM},
        { PixelFormat::BC4SNorm,             DXGI_FORMAT_BC4_SNORM},
        { PixelFormat::BC5UNorm,             DXGI_FORMAT_BC5_UNORM},
        { PixelFormat::BC5SNorm,             DXGI_FORMAT_BC5_SNORM},
        { PixelFormat::BC6HS16,              DXGI_FORMAT_BC6H_SF16},
        { PixelFormat::BC6HU16,              DXGI_FORMAT_BC6H_UF16},
        { PixelFormat::BC7UNorm,             DXGI_FORMAT_BC7_UNORM},
        { PixelFormat::BC7UNormSrgb,         DXGI_FORMAT_BC7_UNORM_SRGB},
    };

    static_assert(_countof(s_DxgiFormatDesc) == (uint32_t)PixelFormat::BC7UNormSrgb + 1, "DXGI format desc table has a wrong size");
}
