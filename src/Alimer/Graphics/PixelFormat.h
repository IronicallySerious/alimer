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
        Unknown,
        // 8-bit pixel formats
        A8UNorm,
        R8UNorm,
        R8SNorm,
        R8UInt,
        R8SInt,

        // 16-bit pixel formats
        R16UNorm,
        R16SNorm,
        R16UInt,
        R16SInt,
        R16Float,
        RG8UNorm,
        RG8SNorm,
        RG8UInt,
        RG8SInt,

        // Packed 16-bit pixel formats
        R5G6B5UNorm,
        RGBA4UNorm,

        // 32-bit pixel formats
        R32UInt,
        R32SInt,
        R32Float,
        RG16UNorm,
        RG16SNorm,
        RG16UInt,
        RG16SInt,
        RG16Float,
        RGBA8UNorm,
        RGBA8UNormSrgb,
        RGBA8SNorm,
        RGBA8UInt,
        RGBA8SInt,
        BGRA8UNorm,
        BGRA8UNormSrgb,

        // Packed 32-Bit Pixel formats
        RGB10A2UNorm,
        RGB10A2UInt,
        RG11B10Float,
        RGB9E5Float,

        // Depth-stencil formats
        D32Float,
        D16UNorm,
        D24UNormS8,
        D32FloatS8,

        // Compressed formats
        BC1UNorm,       // DXT1
        BC1UNormSrgb,
        BC2UNorm,   // DXT3
        BC2UNormSrgb,
        BC3UNorm,   // DXT5
        BC3UNormSrgb,
        BC4UNorm,   // RGTC Unsigned Red
        BC4SNorm,   // RGTC Signed Red
        BC5UNorm,   // RGTC Unsigned RG
        BC5SNorm,   // RGTC Signed RG
        BC6HS16,
        BC6HU16,
        BC7UNorm,
        BC7UNormSrgb,
        Count
    };

    /**
    * Pixel format Type
    */
    enum class PixelFormatType
    {
        /// Unknown format Type
        Unknown,
        /// Floating-point formats
        Float,
        /// Unsigned normalized formats
        UNorm,
        /// Unsigned normalized SRGB formats
        UNormSrgb,
        /// Signed normalized formats
        SNorm,
        /// Unsigned integer formats
        UInt,
        /// Signed integer formats
        SInt
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

    extern ALIMER_API const PixelFormatDesc FormatDesc[];

    ALIMER_API const String& EnumToString(PixelFormat format);

    /**
    * Check if the format has a depth component
    */
    inline bool IsDepthFormat(PixelFormat format)
    {
        assert(FormatDesc[(uint32_t)format].format == format);
        return FormatDesc[(uint32_t)format].isDepth;
    }

    /**
    * Check if the format has a stencil component
    */
    inline bool IsStencilFormat(PixelFormat format)
    {
        assert(FormatDesc[(uint32_t)format].format == format);
        return FormatDesc[(uint32_t)format].isStencil;
    }

    /**
    * Check if the format has depth or stencil components
    */
    inline bool IsDepthStencilFormat(PixelFormat format)
    {
        return IsDepthFormat(format) || IsStencilFormat(format);
    }

    /**
    * Check if the format is a compressed format
    */
    inline bool IsCompressed(PixelFormat format)
    {
        assert(FormatDesc[(uint32_t)format].format == format);
        return FormatDesc[(uint32_t)format].isCompressed;
    }

    ALIMER_API uint32_t GetPixelFormatSize(PixelFormat format);

    /// Calculate the data size of an image level.
    ALIMER_API uint32_t CalculateDataSize(uint32_t width, uint32_t height, PixelFormat format, uint32_t* numRows = 0, uint32_t* rowPitch = 0);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
