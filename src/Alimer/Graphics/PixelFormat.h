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

#include "../Base/String.h"
#include "../vgpu/vgpu.h"
#include <cassert>
#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) 
#endif

namespace alimer
{
    /// Defines pixel format.
    enum class PixelFormat : uint32_t
    {
        Unknown = VGPU_PIXEL_FORMAT_UNKNOWN,
        // 8-bit pixel formats
        A8UNorm = VGPU_PIXEL_FORMAT_A8_UNORM,
        R8UNorm = VGPU_PIXEL_FORMAT_R8_UNORM,
        R8SNorm = VGPU_PIXEL_FORMAT_R8_SNORM,
        R8UInt = VGPU_PIXEL_FORMAT_R8_UINT,
        R8SInt = VGPU_PIXEL_FORMAT_R8_SINT,

        // 16-bit pixel formats
        R16UNorm = VGPU_PIXEL_FORMAT_R16_UNORM,
        R16SNorm = VGPU_PIXEL_FORMAT_R16_SNORM,
        R16UInt = VGPU_PIXEL_FORMAT_R16_UINT,
        R16SInt = VGPU_PIXEL_FORMAT_R16_SINT,
        R16Float = VGPU_PIXEL_FORMAT_R16_FLOAT,
        RG8UNorm = VGPU_PIXEL_FORMAT_RG8_UNORM,
        RG8SNorm = VGPU_PIXEL_FORMAT_RG8_SNORM,
        RG8UInt = VGPU_PIXEL_FORMAT_RG8_UINT,
        RG8SInt = VGPU_PIXEL_FORMAT_RG8_SINT,

        // Packed 16-bit pixel formats
        R5G6B5UNorm = VGPU_PIXEL_FORMAT_R5G6B5_UNORM,
        RGBA4UNorm = VGPU_PIXEL_FORMAT_RGBA4_UNORM,

        // 32-bit pixel formats
        R32UInt = VGPU_PIXEL_FORMAT_R32_UINT,
        R32SInt = VGPU_PIXEL_FORMAT_R32_SINT,
        R32Float = VGPU_PIXEL_FORMAT_R32_FLOAT,
        RG16UNorm = VGPU_PIXEL_FORMAT_RG16_UNORM,
        RG16SNorm = VGPU_PIXEL_FORMAT_RG16_SNORM,
        RG16UInt = VGPU_PIXEL_FORMAT_RG16_UINT,
        RG16SInt = VGPU_PIXEL_FORMAT_RG16_SINT,
        RG16Float = VGPU_PIXEL_FORMAT_RG16_FLOAT,
        RGBA8UNorm = VGPU_PIXEL_FORMAT_RGBA8_UNORM,
        RGBA8SNorm = VGPU_PIXEL_FORMAT_RGBA8_SNORM,
        RGBA8UInt = VGPU_PIXEL_FORMAT_RGBA8_UINT,
        RGBA8SInt = VGPU_PIXEL_FORMAT_RGBA8_SINT,
        BGRA8UNorm = VGPU_PIXEL_FORMAT_BGRA8_UNORM,

        // Packed 32-Bit Pixel formats
        RGB10A2UNorm = VGPU_PIXEL_FORMAT_RGB10A2_UNORM,
        RGB10A2UInt = VGPU_PIXEL_FORMAT_RGB10A2_UINT,
        RG11B10Float = VGPU_PIXEL_FORMAT_RG11B10_FLOAT,
        RGB9E5Float = VGPU_PIXEL_FORMAT_RGB9E5_FLOAT,

        // 64-Bit Pixel Formats
        RG32UInt = VGPU_PIXEL_FORMAT_RG32_UINT,
        RG32SInt = VGPU_PIXEL_FORMAT_RG32_SINT,
        RG32Float = VGPU_PIXEL_FORMAT_RG32_FLOAT,
        RGBA16UNorm = VGPU_PIXEL_FORMAT_RGBA16_UNORM,
        RGBA16SNorm =VGPU_PIXEL_FORMAT_RGBA16_SNORM,
        RGBA16UInt =VGPU_PIXEL_FORMAT_RGBA16_UINT,
        RGBA16SInt =VGPU_PIXEL_FORMAT_RGBA16_SINT,
        RGBA16F = VGPU_PIXEL_FORMAT_RGBA16_FLOAT,

        // 128-Bit Pixel Formats
        RGBA32UInt = VGPU_PIXEL_FORMAT_RGBA32_UINT,
        RGBA32SInt = VGPU_PIXEL_FORMAT_RGBA32_SINT,
        RGBA32Float = VGPU_PIXEL_FORMAT_RGBA32_FLOAT,

        // Depth-stencil formats
        D16 = VGPU_PIXEL_FORMAT_D16,
        D24 = VGPU_PIXEL_FORMAT_D24,
        D24S8 = VGPU_PIXEL_FORMAT_D24S8,
        D32 = VGPU_PIXEL_FORMAT_D32,
        D16F = VGPU_PIXEL_FORMAT_D16F,
        D24F = VGPU_PIXEL_FORMAT_D24F,
        D32F = VGPU_PIXEL_FORMAT_D32F,
        D32FS8 = VGPU_PIXEL_FORMAT_D32FS8,
        D0S8 = VGPU_PIXEL_FORMAT_D0S8,

        // Compressed formats
        BC1UNorm = VGPU_PIXEL_FORMAT_BC1_UNORM,     
        BC2UNorm = VGPU_PIXEL_FORMAT_BC1_UNORM,
        BC3UNorm = VGPU_PIXEL_FORMAT_BC3_UNORM,
        BC4UNorm = VGPU_PIXEL_FORMAT_BC4_UNORM,
        BC4SNorm = VGPU_PIXEL_FORMAT_BC4_UNORM,
        BC5UNorm = VGPU_PIXEL_FORMAT_BC5_UNORM,
        BC5SNorm = VGPU_PIXEL_FORMAT_BC5_SNORM,
        BC6HS16 = VGPU_PIXEL_FORMAT_BC6HS16,
        BC6HU16 = VGPU_PIXEL_FORMAT_BC6HU16,
        BC7UNorm = VGPU_PIXEL_FORMAT_BC7_UNORM,

        // Compressed PVRTC Pixel Formats
        PVRTC_RGB2 = VGPU_PIXEL_FORMAT_PVRTC_RGB2,
        PVRTC_RGBA2 = VGPU_PIXEL_FORMAT_PVRTC_RGBA2,
        PVRTC_RGB4 = VGPU_PIXEL_FORMAT_PVRTC_RGB4,
        PVRTC_RGBA4 = VGPU_PIXEL_FORMAT_PVRTC_RGBA4,

        // Compressed ETC Pixel Formats
        ETC2_RGB8 = VGPU_PIXEL_FORMAT_ETC2_RGB8,
        ETC2_RGB8A1 = VGPU_PIXEL_FORMAT_ETC2_RGB8A1,

        // Compressed ASTC Pixel Formats
        ASTC4x4 = VGPU_PIXEL_FORMAT_ASTC4x4,
        ASTC5x5 = VGPU_PIXEL_FORMAT_ASTC5x5,
        ASTC6x6 = VGPU_PIXEL_FORMAT_ASTC6x6,
        ASTC8x5 = VGPU_PIXEL_FORMAT_ASTC8x5,
        ASTC8x6 = VGPU_PIXEL_FORMAT_ASTC8x6,
        ASTC8x8 = VGPU_PIXEL_FORMAT_ASTC8x8,
        ASTC10x10 = VGPU_PIXEL_FORMAT_ASTC10x10,
        ASTC12x12 = VGPU_PIXEL_FORMAT_ASTC12x12,

        Count = VGPU_PIXEL_FORMAT_COUNT
    };

    /**
    * Pixel format Type
    */
    enum class PixelFormatType
    {
        /// Unknown format Type.
        Unknown = VGPU_PIXEL_FORMAT_TYPE_UNKNOWN,
        /// Floating-point formats.
        Float = VGPU_PIXEL_FORMAT_TYPE_FLOAT,
        /// Unsigned normalized formats.
        UNorm = VGPU_PIXEL_FORMAT_TYPE_UNORM,
        /// Signed normalized formats.
        SNorm = VGPU_PIXEL_FORMAT_TYPE_SNORM,
        /// Unsigned integer formats.
        UInt = VGPU_PIXEL_FORMAT_TYPE_UINT,
        /// Signed integer formats.
        SInt = VGPU_PIXEL_FORMAT_TYPE_SINT
    };

    struct PixelFormatDesc
    {
        PixelFormat format;
        const String name;
        uint32_t bytesPerBlock;
        uint32_t channelCount;
        PixelFormatType Type;
        struct
        {
            bool isDepth;
            bool isStencil;
            bool isCompressed;
        };
        struct
        {
            uint32_t width;
            uint32_t height;
        } compressionRatio;
    };

    ALIMER_API const String& GetFormatName(PixelFormat format);

    /// Get the number of bytes per format.
    inline uint32_t GetFormatBitsPerPixel(PixelFormat format)
    {
        return vgpuGetFormatBitsPerPixel((VgpuPixelFormat)format);
    }

    inline uint32_t GetFormatBlockSize(PixelFormat format)
    {
        return vgpuGetFormatBlockSize((VgpuPixelFormat)format);
    }

    /// Check if the format has a depth component
    inline bool IsDepthFormat(PixelFormat format)
    {
        return vgpuIsDepthFormat((VgpuPixelFormat)format);
    }

    /// Check if the format has a stencil component
    inline bool IsStencilFormat(PixelFormat format)
    {
        return vgpuIsStencilFormat((VgpuPixelFormat)format);
    }

    /// Check if the format has depth or stencil components
    inline bool IsDepthStencilFormat(PixelFormat format)
    {
        return vgpuIsDepthStencilFormat((VgpuPixelFormat)format);
    }

    /// Check if the format is a compressed format
    inline bool IsCompressedFormat(PixelFormat format)
    {
        return vgpuIsCompressedFormat((VgpuPixelFormat)format);
    }

    /// Get the format compression ration along the x-axis
    inline uint32_t GetFormatBlockWidth(PixelFormat format)
    {
        return vgpuGetFormatBlockWidth((VgpuPixelFormat)format);
    }

    /// Get the format compression ration along the y-axis
    inline uint32_t GetFormatBlockHeight(PixelFormat format)
    {
        return vgpuGetFormatBlockHeight((VgpuPixelFormat)format);
    }

    /// Get the format Type
    inline PixelFormatType GetFormatType(PixelFormat format)
    {
        return static_cast<PixelFormatType>(vgpuGetFormatType((VgpuPixelFormat)format));
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
