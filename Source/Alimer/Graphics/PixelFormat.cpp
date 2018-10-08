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
#include "../Core/Log.h"

namespace Alimer
{
    bool IsDepthFormat(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::Depth16UNorm:
        case PixelFormat::Depth24UNormStencil8:
        case PixelFormat::Depth32Float:
        case PixelFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
        }
    }

    bool IsStencilFormat(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::Depth24UNormStencil8:
        case PixelFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
        }
    }

    bool IsDepthStencilFormat(PixelFormat format)
    {
        return IsDepthFormat(format) || IsStencilFormat(format);
    }

    bool IsCompressed(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::BC1:
        case PixelFormat::BC2:
        case PixelFormat::BC3:
        case PixelFormat::BC4UNorm:
        case PixelFormat::BC4SNorm:
        case PixelFormat::BC5UNorm:
        case PixelFormat::BC5SNorm:
        case PixelFormat::BC6HSFloat:
        case PixelFormat::BC6HUFloat:
            return true;
        default:
            return false;
        }
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
            ALIMER_LOGERROR("Invalid PixelFormat value");
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
