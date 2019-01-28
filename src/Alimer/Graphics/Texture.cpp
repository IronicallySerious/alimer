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

#include "../Graphics/Texture.h"
#include "../Graphics/GraphicsDevice.h"
#include "../IO/Stream.h"
#include "../Math/MathUtil.h"
#include "../Graphics/Backend.h"
#include "Core/Log.h"

namespace alimer
{
    Texture::Texture(GraphicsDevice* device, const TextureDescriptor* descriptor)
        : GPUResource(device, Type::Texture)
        , _width(descriptor->width)
        , _height(descriptor->height)
        , _depth(descriptor->depth)
        , _arraySize(descriptor->arraySize)
        , _mipLevels(descriptor->mipLevels)
        , _samples(descriptor->samples)
        , _type(descriptor->type)
        , _format(descriptor->format)
        , _usage(descriptor->usage)
    {
    }

    Texture::Texture(GraphicsDevice* device)
        : GPUResource(device, Type::Texture)
    {

    }

    Texture::~Texture()
    {
        Destroy();
    }

    void Texture::Destroy()
    {
        // Destroy backend.
        PlatformDestroy();
    }

    void Texture::DefineFromHandle(GPUTexture* handle,
        TextureType type,
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t arraySize,
        uint32_t mipLevels,
        PixelFormat format,
        TextureUsage usage,
        SampleCount sampleCount)
    {
        ALIMER_ASSERT_MSG(handle != nullptr, "Invalid backend handle.");
        ALIMER_ASSERT_MSG(width >= 1, "Width must be greather than 0.");
        ALIMER_ASSERT_MSG(height >= 1, "Height must be greather than 0.");
        ALIMER_ASSERT_MSG(arraySize >= 1 && arraySize <= 2048, "Array size must be between 1 and 2048.");
        ALIMER_ASSERT_MSG(format != PixelFormat::Unknown, "Invalid pixel format.");
        ALIMER_ASSERT_MSG(usage != TextureUsage::None, "Invalid pixel format.");

        switch (type)
        {
        case TextureType::Type1D:
            assert(height == 1 && depth == 1 && sampleCount == SampleCount::Count1);
            break;
        case TextureType::Type2D:
            assert(depth == 1);
            break;
        case TextureType::Type3D:
            assert(sampleCount == SampleCount::Count1);
            break;
        case TextureType::TypeCube:
            assert(depth == 1 && sampleCount == SampleCount::Count1);
            break;
        }

        _handle = handle;
        _type = type;
        _width = width;
        _height = height;
        _depth = depth;
        _arraySize = arraySize;
        _mipLevels = mipLevels;
        if (_mipLevels == MaxPossible)
        {
            uint32_t dims = width | height | _depth;
            _mipLevels = bitScanReverse(dims) + 1;
        }

        _format = format;
        _usage = usage;
        _samples = sampleCount;
    }
}
