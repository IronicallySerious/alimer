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

#include "../Graphics/Texture.h"
#include "../Graphics/Graphics.h"
#include "../IO/Stream.h"
#include "../Core/Log.h"

namespace Alimer
{
    Texture::Texture(GPUDevice* device, const TextureDescriptor* descriptor)
        : GraphicsResource(device)
    {
        memcpy(&_descriptor, descriptor, sizeof(TextureDescriptor));
    }

    bool Texture::Define2D(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels, uint32_t arrayLayers,
        const ImageLevel* initialData, TextureUsage usage, SampleCount samples)
    {
#if defined(ALIMER_DEV)
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
            || arrayLayers == 0
            || mipLevels == 0)
        {
            ALIMER_LOGCRITICAL("Cannot create an empty texture");
        }
#endif // defined(ALIMER_DEV)

        // Destroy old texture.
        Destroy();

        // Create new instance.
        _descriptor.type = TextureType::Type2D;
        _descriptor.usage = usage;
        _descriptor.format = format;
        _descriptor.width = width;
        _descriptor.height = height;
        _descriptor.depth = 1;
        _descriptor.mipLevels = mipLevels;
        _descriptor.arrayLayers = arrayLayers;
        _descriptor.samples = samples;

        //_texture = _device->GetGPUDevice()->CreateTexture(&_descriptor, initialData);
        //if (_texture == nullptr)
        //{
            ALIMER_LOGERROR("Failed to define sampler.");
        //    return false;
        //}

        //ALIMER_LOGDEBUG("Texture defined with success.");
        //return true;
    }

    void Texture::RegisterObject()
    {
        //RegisterFactory<Texture>();
    }

    bool Texture::BeginLoad(Stream& source)
    {
        ALIMER_UNUSED(source);
        return true;
    }

    bool Texture::EndLoad()
    {
        return true;
    }
}
