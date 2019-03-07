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

#include "../Resource/Image.h"
#include "../Core/Log.h"
#include "../IO/Stream.h"
#include <stb_image.h>
#include <stb_image_write.h>

namespace alimer
{
    Image::Image()
    {
    }

    void Image::Define(const uvec2& newSize, PixelFormat newFormat)
    {
        /*if (all(equal(newSize, _size)) && newFormat == _format) {
            return;
        }*/

        const uint32_t formatSize = GetFormatBitsPerPixel(newFormat);
        if (formatSize == 0)
        {
            ALIMER_LOGERROR("Can not set image size with unspecified pixel byte size (including compressed formats)");
            return;
        }

        _memorySize = newSize.x * newSize.y * formatSize;
        _data = std::make_unique< uint8_t[]>(_memorySize);
        _size = newSize;
        _format = newFormat;
        _mipLevels = 1;
    }

    void Image::SetData(const uint8_t* pixelData)
    {
        if (!IsCompressedFormat(_format))
        {
            memcpy(_data.get(), pixelData, _memorySize);
        }
        else
        {
            ALIMER_LOGERROR("Can not set pixel data of a compressed image");
        }
    }

    void StbiWriteCallback(void *context, void *data, int len)
    {
        Stream* stream = reinterpret_cast<Stream*>(context);
        stream->Write(data, len);
    }

    bool Image::Save(Stream* dest, ImageFormat format) const
    {
        //ALIMER_PROFILE(SaveImageBMP);
        ALIMER_ASSERT(dest);

        if (IsCompressedFormat(_format))
        {
            ALIMER_LOGERROR("Can not save compressed image '{}'", GetName());
            return false;
        }

        if (!_data)
        {
            ALIMER_LOGERROR("Can not save zero-sized image '{}'", GetName());
            return false;
        }

        uint32_t components = GetFormatBitsPerPixel(_format) / 4;
        if (components < 1 || components > 4)
        {
            ALIMER_LOGERROR("Unsupported pixel format for PNG save on image '{}'", GetName());
            return false;
        }

        switch (format)
        {
        case ImageFormat::Bmp:
            return SaveBmp(dest);
        case ImageFormat::Png:
            return SavePng(dest);
        default:
            return false;
        }
    }

    bool Image::SaveBmp(Stream* dest) const
    {
        uint32_t components = GetFormatBitsPerPixel(_format) / 4; 

        return stbi_write_bmp_to_func(
            StbiWriteCallback,
            dest,
            static_cast<int>(_size.x),
            static_cast<int>(_size.y),
            static_cast<int>(components),
            _data.get()) != 0;
    }

    bool Image::SavePng(Stream* dest) const
    {
        uint32_t components = GetFormatBitsPerPixel(_format) / 4;

        return stbi_write_png_to_func(
            StbiWriteCallback,
            dest,
            static_cast<int>(_size.x),
            static_cast<int>(_size.y),
            static_cast<int>(components),
            _data.get(),
            0) != 0;
    }

    void Image::RegisterObject()
    {
        RegisterFactory<Image>();
    }
}
