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
#include "../Graphics/GPUDevice.h"
#include "../Graphics/GPUDeviceImpl.h"
#include "../IO/Stream.h"
#include "../Core/Log.h"

namespace alimer
{
    Texture::Texture()
        : GPUResource(nullptr, Type::Texture)
        , _descriptor{}
    {
    }

    Texture::Texture(GPUTexture* texture)
        : GPUResource(nullptr, Type::Texture)
        , _texture(texture)
        , _descriptor(texture->GetDescriptor())
    {
    }

    Texture::~Texture()
    {
        Destroy();
    }

    void Texture::Destroy()
    {
        SafeDelete(_texture);
    }

    bool Texture::CreateGPUTexture(const void* initialData)
    {
        // Destroy old instance first.
        Destroy();

        _texture = _device->GetImpl()->CreateTexture(_descriptor, initialData);
        return _texture != nullptr;
    }

    /* Texture2D */
    Texture2D::Texture2D()
        : Texture()
    {

    }

    Texture2D::Texture2D(GPUTexture* texture)
        : Texture(texture)
    {

    }

    bool Texture2D::Define(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arraySize,
        PixelFormat format, TextureUsage usage, SampleCount samples, const void* initialData)
    {
        if (format == PixelFormat::Unknown)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (usage == TextureUsage::None)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (width == 0
            || height == 0
            || arraySize == 0
            || mipLevels == 0)
        {
            ALIMER_LOGCRITICAL("Cannot create an empty texture");
        }

        _descriptor.width = width;;
        _descriptor.height = height;
        _descriptor.depth = 1;
        _descriptor.arraySize = arraySize;
        _descriptor.mipLevels = mipLevels;
        _descriptor.samples = static_cast<SampleCount>(samples);
        _descriptor.type = TextureType::Type2D;
        _descriptor.format = format;
        _descriptor.usage = usage;
        return CreateGPUTexture(initialData);
    }

}
