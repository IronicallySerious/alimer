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

#include "Graphics/Texture.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/DeviceBackend.h"
#include "IO/Stream.h"
#include "Math/MathUtil.h"
#include "Core/Log.h"

namespace alimer
{
    Texture::Texture() 
    {
    }

    Texture::~Texture()
    {
        Destroy();
    }

    bool Texture::CreateGPUTexture( void* nativeTexture, const void* initialData)
    {
        // Destroy old instance first.
        Destroy();

        TextureDescriptor descriptor;
        descriptor.width = _width;
        descriptor.height = _height;
        descriptor.depth = _depth;
        descriptor.arraySize = _arraySize;
        descriptor.mipLevels = _mipLevels;
        descriptor.samples = _samples;
        descriptor.type = _type;
        descriptor.format = _format;
        descriptor.usage = _usage;
        _texture = gGraphics().GetImpl()->CreateTexture(&descriptor, nativeTexture, initialData);
        return _texture != nullptr;
    }

    void Texture::Destroy()
    {
        SafeDelete(_texture);
    }

    void Texture::RegisterObject()
    {
        RegisterFactory<Texture>();
    }

    bool Texture::Define2D(
        uint32_t width, 
        uint32_t height, 
        PixelFormat format, 
        uint32_t arraySize, 
        uint32_t mipLevels, 
        const void* pInitData, 
        TextureUsage usage)
    {
        ALIMER_ASSERT_MSG(width >= 1, "Width must be greather than 0.");
        ALIMER_ASSERT_MSG(height >= 1, "Height must be greather than 0.");
        ALIMER_ASSERT_MSG(arraySize >= 1 && arraySize <= 2048, "Array size must be between 1 and 2048.");
        ALIMER_ASSERT_MSG(format != PixelFormat::Unknown, "Invalid pixel format.");

        _width = width;
        _height = height;
        _depth = 1;
        _arraySize = arraySize;
        _mipLevels = mipLevels;
        if (_mipLevels == MaxPossible)
        {
            uint32_t dims = width | height | _depth;
            _mipLevels = bitScanReverse(dims) + 1;
        }

        _samples = SampleCount::Count1;
        _type = TextureType::Type2D;
        _format = format;
        _usage = usage;
        return CreateGPUTexture(nullptr, pInitData);
    }


    bool Texture::Define2DMultisample(
        uint32_t width, 
        uint32_t height, 
        PixelFormat format, 
        SampleCount samples,
        uint32_t arraySize, 
        TextureUsage usage,
        const void* pInitData)
    {
        ALIMER_ASSERT_MSG(width >= 1, "Width must be greather than 0.");
        ALIMER_ASSERT_MSG(height >= 1, "Height must be greather than 0.");
        ALIMER_ASSERT_MSG(arraySize >= 1 && arraySize <= 2048, "Array size must be between 1 and 2048.");
        ALIMER_ASSERT_MSG(_format != PixelFormat::Unknown, "Invalid pixel format.");

        _width = width;
        _height = height;
        _depth = 1;
        _arraySize = arraySize;
        _mipLevels = 1;
        _samples = samples;
        _type = TextureType::Type2D;
        _format = format;
        _usage = usage;
        return CreateGPUTexture(nullptr, pInitData);
    }
}
