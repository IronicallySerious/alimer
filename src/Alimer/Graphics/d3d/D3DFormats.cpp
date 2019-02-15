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

#include "D3DConvert.h"

namespace alimer
{
    const DxgiFormatDesc s_DxgiFormatDesc[] =
    {
        {PixelFormat::Undefined,                DXGI_FORMAT_UNKNOWN},
        // 8-bit pixel formats
        {PixelFormat::A8UNorm,                  DXGI_FORMAT_A8_UNORM},
        {PixelFormat::R8UNorm,                  DXGI_FORMAT_R8_UNORM},
        {PixelFormat::R8SNorm,                  DXGI_FORMAT_R8_SNORM},
        {PixelFormat::R8UInt,                   DXGI_FORMAT_R8_UINT},
        {PixelFormat::R8SInt,                   DXGI_FORMAT_R8_SINT},

        // 16-bit pixel formats
        {PixelFormat::R16UNorm,                 DXGI_FORMAT_R16_UNORM},
        {PixelFormat::R16SNorm,                 DXGI_FORMAT_R16_SNORM},
        {PixelFormat::R16UInt,                  DXGI_FORMAT_R16_UINT},
        {PixelFormat::R16SInt,                  DXGI_FORMAT_R16_SINT},
        {PixelFormat::R16Float,                 DXGI_FORMAT_R16_FLOAT},
        {PixelFormat::RG8UNorm,                 DXGI_FORMAT_R8G8_UNORM},
        {PixelFormat::RG8SNorm,                 DXGI_FORMAT_R8G8_SNORM},
        {PixelFormat::RG8UInt,                  DXGI_FORMAT_R8G8_UINT},
        {PixelFormat::RG8SInt,                  DXGI_FORMAT_R8G8_SINT},

        // Packed 16-bit pixel formats
        {PixelFormat::R5G6B5UNorm,              DXGI_FORMAT_B5G6R5_UNORM},
        {PixelFormat::RGBA4UNorm,               DXGI_FORMAT_B4G4R4A4_UNORM},

        // 32-bit pixel formats
        {PixelFormat::R32UInt,                  DXGI_FORMAT_R32_UINT},
        {PixelFormat::R32SInt,                  DXGI_FORMAT_R32_SINT},
        {PixelFormat::R32Float,                 DXGI_FORMAT_R32_FLOAT},
        {PixelFormat::RG16UNorm,                DXGI_FORMAT_R16G16_UNORM},
        {PixelFormat::RG16SNorm,                DXGI_FORMAT_R16G16_SNORM},
        {PixelFormat::RG16UInt,                 DXGI_FORMAT_R16G16_UINT},
        {PixelFormat::RG16SInt,                 DXGI_FORMAT_R16G16_SINT},
        {PixelFormat::RG16Float,                DXGI_FORMAT_R16G16_FLOAT},
        {PixelFormat::RGBA8UNorm,               DXGI_FORMAT_R8G8B8A8_UNORM},
        {PixelFormat::RGBA8UNormSrgb,           DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
        {PixelFormat::RGBA8SNorm,               DXGI_FORMAT_R8G8B8A8_SNORM},
        {PixelFormat::RGBA8UInt,                DXGI_FORMAT_R8G8B8A8_UINT},
        {PixelFormat::RGBA8SInt,                DXGI_FORMAT_R8G8B8A8_SINT},
        {PixelFormat::BGRA8UNorm,               DXGI_FORMAT_B8G8R8A8_UNORM},
        {PixelFormat::BGRA8UNormSrgb,           DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},

        // Packed 32-Bit Pixel formats
        {PixelFormat::RGB10A2UNorm,             DXGI_FORMAT_R10G10B10A2_UNORM},
        {PixelFormat::RGB10A2UInt,              DXGI_FORMAT_R10G10B10A2_UINT},
        {PixelFormat::RG11B10Float,             DXGI_FORMAT_R11G11B10_FLOAT},
        {PixelFormat::RGB9E5Float,              DXGI_FORMAT_R9G9B9E5_SHAREDEXP},

        // 64-Bit Pixel Formats
        {PixelFormat::RG32UInt,                 DXGI_FORMAT_R32G32_UINT},
        {PixelFormat::RG32SInt,                 DXGI_FORMAT_R32G32_SINT},
        {PixelFormat::RG32Float,                DXGI_FORMAT_R32G32_FLOAT},
        {PixelFormat::RGBA16UNorm,              DXGI_FORMAT_R16G16B16A16_UNORM},
        {PixelFormat::RGBA16SNorm,              DXGI_FORMAT_R16G16B16A16_SNORM},
        {PixelFormat::RGBA16UInt,               DXGI_FORMAT_R16G16B16A16_UINT},
        {PixelFormat::RGBA16SInt,               DXGI_FORMAT_R16G16B16A16_SINT},
        {PixelFormat::RGBA16Float,              DXGI_FORMAT_R16G16B16A16_FLOAT},

        // 128-Bit Pixel Formats
        {PixelFormat::RGBA32UInt,               DXGI_FORMAT_R32G32B32A32_UINT},
        {PixelFormat::RGBA32SInt,               DXGI_FORMAT_R32G32B32A32_SINT},
        {PixelFormat::RGBA32Float,              DXGI_FORMAT_R32G32B32A32_FLOAT},

        // Depth-stencil formats
        {PixelFormat::Depth16UNorm,             DXGI_FORMAT_D16_UNORM},
        {PixelFormat::Depth32Float,             DXGI_FORMAT_D32_FLOAT},
        {PixelFormat::Depth24UNormStencil8,     DXGI_FORMAT_D24_UNORM_S8_UINT},
        {PixelFormat::Depth32FloatStencil8,     DXGI_FORMAT_D32_FLOAT_S8X24_UINT},
        {PixelFormat::Stencil8,                 DXGI_FORMAT_D24_UNORM_S8_UINT},

        // Compressed BC formats
        { PixelFormat::BC1UNorm,                DXGI_FORMAT_BC1_UNORM},
        { PixelFormat::BC1UNormSrgb,            DXGI_FORMAT_BC1_UNORM_SRGB},
        { PixelFormat::BC2UNorm,                DXGI_FORMAT_BC2_UNORM},
        { PixelFormat::BC2UNormSrgb,            DXGI_FORMAT_BC2_UNORM_SRGB},
        { PixelFormat::BC3UNorm,                DXGI_FORMAT_BC3_UNORM},
        { PixelFormat::BC3UNormSrgb,            DXGI_FORMAT_BC3_UNORM_SRGB},
        { PixelFormat::BC4UNorm,                DXGI_FORMAT_BC4_UNORM},
        { PixelFormat::BC4SNorm,                DXGI_FORMAT_BC4_SNORM},
        { PixelFormat::BC5UNorm,                DXGI_FORMAT_BC5_UNORM},
        { PixelFormat::BC5SNorm,                DXGI_FORMAT_BC5_SNORM},
        { PixelFormat::BC6HS16,                 DXGI_FORMAT_BC6H_SF16},
        { PixelFormat::BC6HU16,                 DXGI_FORMAT_BC6H_UF16},
        { PixelFormat::BC7UNorm,                DXGI_FORMAT_BC7_UNORM},
        { PixelFormat::BC7UNormSrgb,            DXGI_FORMAT_BC7_UNORM_SRGB},

        // Compressed PVRTC Pixel Formats
        { PixelFormat::PVRTC_RGB2,              DXGI_FORMAT_UNKNOWN},
        { PixelFormat::PVRTC_RGBA2,             DXGI_FORMAT_UNKNOWN},
        { PixelFormat::PVRTC_RGB4,              DXGI_FORMAT_UNKNOWN},
        { PixelFormat::PVRTC_RGBA4,             DXGI_FORMAT_UNKNOWN},

        // Compressed ETC Pixel Formats
        { PixelFormat::ETC2_RGB8,               DXGI_FORMAT_UNKNOWN},
        { PixelFormat::ETC2_RGB8Srgb,           DXGI_FORMAT_UNKNOWN},
        { PixelFormat::ETC2_RGB8A1,             DXGI_FORMAT_UNKNOWN},
        { PixelFormat::ETC2_RGB8A1Srgb,         DXGI_FORMAT_UNKNOWN},

        // Compressed ASTC Pixel Formats
        { PixelFormat::ASTC4x4,                 DXGI_FORMAT_UNKNOWN},
        { PixelFormat::ASTC5x5,                 DXGI_FORMAT_UNKNOWN},
        { PixelFormat::ASTC6x6,                 DXGI_FORMAT_UNKNOWN},
        { PixelFormat::ASTC8x5,                 DXGI_FORMAT_UNKNOWN},
        { PixelFormat::ASTC8x6,                 DXGI_FORMAT_UNKNOWN },
        { PixelFormat::ASTC8x8,                 DXGI_FORMAT_UNKNOWN },
        { PixelFormat::ASTC10x10,               DXGI_FORMAT_UNKNOWN },
        { PixelFormat::ASTC12x12,               DXGI_FORMAT_UNKNOWN },
    };

    static_assert(_countof(s_DxgiFormatDesc) == (uint32_t)PixelFormat::Count, "DXGI format desc table has a wrong size");
}
