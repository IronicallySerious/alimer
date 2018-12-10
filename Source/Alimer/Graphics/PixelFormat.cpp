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

#include "../Graphics/PixelFormat.h"
#include "../Math/MathUtil.h"
#include "../Debug/Log.h"

namespace Alimer
{
    extern ALIMER_API const PixelFormatDesc FormatDesc[] =
    {
        // Format                           Name,               BytesPerBlock, ChannelCount,  Type  {Depth,  Stencil, Compressed},      {CompressionRatio.Width, CompressionRatio.Height}
        { PixelFormat::Unknown,             "Unknown",          0,  0,  PixelFormatType::Unknown,    { false, false, false},            {1, 1}},
        { PixelFormat::R8UNorm,             "R8UNorm",          1,  1,  PixelFormatType::UNorm,      { false, false, false},            {1, 1}},
        { PixelFormat::R8SNorm,             "R8SNorm",          1,  1,  PixelFormatType::SNorm,      { false, false, false},            {1, 1}},
        { PixelFormat::R16UNorm,            "R16UNorm",         2,  1,  PixelFormatType::UNorm,      { false, false, false},            {1, 1}},
        { PixelFormat::R16SNorm,            "R16SNorm",         2,  1,  PixelFormatType::SNorm,      { false, false, false},            {1, 1}},
        { PixelFormat::RG8UNorm,            "RG8UNorm",         2,  2,  PixelFormatType::UNorm,      { false, false, false},            {1, 1}},
        { PixelFormat::RG8SNorm,            "RG8SNorm",         2,  2,  PixelFormatType::SNorm,      { false, false, false,},            {1, 1}},
        { PixelFormat::RG16UNorm,           "RG16UNorm",        4,  2,  PixelFormatType::UNorm,      { false, false, false},            {1, 1}},
        { PixelFormat::RG16SNorm,           "RG16SNorm",        4,  2,  PixelFormatType::SNorm,      { false, false, false},            {1, 1}},
        { PixelFormat::RGB16UNorm,          "RGB16UNorm",       6,  3,  PixelFormatType::UNorm,      { false, false, false},            {1, 1}},
        { PixelFormat::RGB16SNorm,          "RGB16SNorm",       6,  3,  PixelFormatType::SNorm,      { false, false, false},            {1, 1}},

        { PixelFormat::RGBA8UNorm,          "RGBA8UNorm",       4,  4,  PixelFormatType::UNorm,      { false, false, false},            {1, 1}},
        { PixelFormat::RGBA8UNormSrgb,      "RGBA8UNormSrgb",   4,  4,  PixelFormatType::UNormSrgb,  { false, false, false},            {1, 1}},
        { PixelFormat::RGBA8SNorm,          "RGBA8SNorm",       4,  4,  PixelFormatType::SNorm,      { false, false, false},            {1, 1}},
                                                                                                       
        { PixelFormat::BGRA8UNorm,          "BGRA8UNorm",       4,  4,  PixelFormatType::UNorm,      { false,  false, false},           {1, 1}},
        { PixelFormat::BGRA8UNormSrgb,      "BGRA8UNormSrgb",   4,  4,  PixelFormatType::UNormSrgb,  { false,  false, false},           {1, 1}},

        { PixelFormat::D32Float,            "D32Float",         4,  1,  PixelFormatType::Float,      { true,   false, false},           {1, 1}},
        { PixelFormat::D16UNorm,            "D16UNorm",         2,  1,  PixelFormatType::UNorm,      { true,   false, false},           {1, 1}},
        { PixelFormat::D24UNormS8,          "D24UNormS8",       4,  2,  PixelFormatType::UNorm,      { true,   true,  false},           {1, 1}},
        { PixelFormat::D32FloatS8,          "D32FloatS8",       8,  2,  PixelFormatType::Float,      { true,   true,  false},           {1, 1}},

        { PixelFormat::BC1UNorm,            "BC1UNorm",         8,  3,  PixelFormatType::UNorm,      { false,  false, true },           {4, 4}},
        { PixelFormat::BC1UNormSrgb,        "BC1UNormSrgb",     8,  3,  PixelFormatType::UNormSrgb,  { false,  false, true },           {4, 4}},
        { PixelFormat::BC2UNorm,            "BC2UNorm",         16, 4,  PixelFormatType::UNorm,      { false,  false, true },           {4, 4}},
        { PixelFormat::BC2UNormSrgb,        "BC2UNormSrgb",     16, 4,  PixelFormatType::UNormSrgb,  { false,  false, true },           {4, 4}},
        { PixelFormat::BC3UNorm,            "BC3UNorm",         16, 4,  PixelFormatType::UNorm,      { false,  false, true },           {4, 4}},
        { PixelFormat::BC3UNormSrgb,        "BC3UNormSrgb",     16, 4,  PixelFormatType::UNormSrgb,  { false,  false, true },           {4, 4}},
        { PixelFormat::BC4UNorm,            "BC4UNorm",         8,  1,  PixelFormatType::UNorm,      { false,  false, true },           {4, 4}},
        { PixelFormat::BC4SNorm,            "BC4SNorm",         8,  1,  PixelFormatType::SNorm,      { false,  false, true },           {4, 4}},
        { PixelFormat::BC5UNorm,            "BC5UNorm",         16, 2,  PixelFormatType::UNorm,      { false,  false, true },           {4, 4}},
        { PixelFormat::BC5SNorm,            "BC5SNorm",         16, 2,  PixelFormatType::SNorm,      { false,  false, true },           {4, 4}},

        { PixelFormat::BC6HS16,             "BC6HS16",          16, 3,  PixelFormatType::Float,      { false,  false, true, },          { 4, 4 }},
        { PixelFormat::BC6HU16,             "BC6HU16",          16, 3,  PixelFormatType::Float,      { false,  false, true, },          { 4, 4 } },
        { PixelFormat::BC7UNorm,            "BC7UNorm",         16, 4,  PixelFormatType::UNorm,      { false,  false, true, },          { 4, 4 } },
        { PixelFormat::BC7UNormSrgb,        "BC7UNormSrgb",     16, 4,  PixelFormatType::UNormSrgb,  { false,  false, true, },          { 4, 4 } },
    };

    const String& EnumToString(PixelFormat format)
    {
        assert(FormatDesc[(uint32_t)format].format == format);
        return FormatDesc[(uint32_t)format].name;
    }

    uint32_t GetPixelFormatSize(PixelFormat format)
    {
        switch (format)
        {
            //case PixelFormat::A8UNorm:
        case PixelFormat::R8UNorm:
            return 1;

        case PixelFormat::RG8UNorm:
            //case PixelFormat::R16UNorm:
            //case PixelFormat::R16Float:
            //case PixelFormat::Depth16UNorm:
            return 2;

        case PixelFormat::RGBA8UNorm:
            //case PixelFormat::RG16UNorm:
            //case PixelFormat::RG16Float:
            //case PixelFormat::R32Float:
            //case PixelFormat::Depth32Float:
            //case PixelFormat::Depth24UNormStencil8:
            return 4;

            //case PixelFormat::RGBA16UNorm:
            //case PixelFormat::RGBA16Float:
            //case PixelFormat::RG32Float:
            //    return 8;
            //case PixelFormat::RGBA32Float:
            //    return 16;

            //case PixelFormat::Stencil8:
            //    return 1;

            //case PixelFormat::BC1:
            //case PixelFormat::BC2:
            //case PixelFormat::BC3:
            //case PixelFormat::ETC1:
            //case PixelFormat::PVRTC_RGB_2BPP:
            //case PixelFormat::PVRTC_RGBA_2BPP:
            //case PixelFormat::PVRTC_RGB_4BPP:
            //case PixelFormat::PVRTC_RGBA_4BPP:
            //    return 0;

        default:
            ALIMER_LOGERROR("GetPixelFormatSize", "Invalid PixelFormat value");
            //ALIMER_LOGERRORF("Invalid PixelFormat value %s", EnumToString(format));
            return 0;
        }
    }

    uint32_t CalculateDataSize(uint32_t width, uint32_t height, PixelFormat format, uint32_t* numRows, uint32_t* rowPitch)
    {
        uint32_t rows, rowSize, dataSize;

        //if (format < PixelFormat::BC1)
        {
            rows = height;
            rowSize = width * GetPixelFormatSize(format);
            dataSize = rows * rowSize;
        }
        /*else if (format < PixelFormat::PVRTC_RGB_2BPP)
        {
            uint32_t blockSize = (format == PixelFormat::BC1 || format == PixelFormat::ETC1) ? 8 : 16;
            rows = (size.height + 3) / 4;
            rowSize = ((size.width + 3) / 4) * blockSize;
            dataSize = rows * rowSize;
        }
        else
        {
            uint32_t blockSize = format < PixelFormat::PVRTC_RGB_4BPP ? 2 : 4;
            uint32_t dataWidth = Max(width, blockSize == 2 ? 16u : 8u);
            rows = Max(height, 8u);
            dataSize = (dataWidth * rows * blockSize + 7) >> 3;
            rowSize = dataSize / rows;
        }*/

        if (numRows)
            *numRows = rows;

        if (rowPitch)
            *rowPitch = rowSize;

        return dataSize;
    }
}
