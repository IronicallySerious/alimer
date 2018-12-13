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
#include "../Debug/Log.h"

namespace Alimer
{
    Texture::Texture(Graphics* graphics)
        : GraphicsResource(graphics)
    {

    }

    bool Texture::Define(const TextureDescriptor* descriptor, const ImageLevel* initialData)
    {
        ALIMER_ASSERT(descriptor);

        if (descriptor->usage == TextureUsage::Unknown)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (descriptor->width == 0
            || descriptor->height == 0
            || descriptor->depth == 0
            || descriptor->arrayLayers == 0
            || descriptor->mipLevels == 0)
        {
            ALIMER_LOGCRITICAL("Cannot create an empty texture");
        }

        _type = descriptor->type;
        _usage = descriptor->usage;
        _format = descriptor->format;
        _width = descriptor->width;
        _height = descriptor->height;
        _depth = descriptor->depth;
        _mipLevels = descriptor->mipLevels;
        _arrayLayers = descriptor->arrayLayers;
        _samples = descriptor->samples;

        Destroy();

        return Create(initialData);
    }

    bool Texture::Define2D(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels, uint32_t arrayLayers,
        const ImageLevel* initialData, TextureUsage usage, SampleCount samples)
    {
#if defined(ALIMER_DEV)
        if (format == PixelFormat::Unknown)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (usage == TextureUsage::Unknown)
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

        _type = TextureType::Type2D;
        _usage = usage;
        _format = format;
        _width = width;
        _height = height;
        _depth = 1;
        _mipLevels = mipLevels;
        _arrayLayers = arrayLayers;
        _samples = samples;

        // Create new 
        return Create(initialData);
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
