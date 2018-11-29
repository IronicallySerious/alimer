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
#include "../Graphics/GraphicsImpl.h"
#include "../IO/Stream.h"
#include "../Debug/Log.h"

namespace Alimer
{
    Texture::Texture()
        : GraphicsResource(Object::GetSubsystem<Graphics>())
        , _impl(nullptr)
    {

    }

    Texture::Texture(TextureImpl* impl)
        : GraphicsResource(Object::GetSubsystem<Graphics>())
        , _impl(impl)
    {
        _type = _impl->type;
        _usage = _impl->usage;
        _format = _impl->format;
        _width = _impl->width;
        _height = _impl->height;
        _depth = _impl->depth;
        _mipLevels = _impl->depth;
        _arrayLayers = _impl->arrayLayers;
        _samples = _impl->samples;
    }

    Texture::~Texture()
    {
        Destroy();
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
        _mipLevels = descriptor->depth;
        _arrayLayers = descriptor->arrayLayers;
        _samples = descriptor->samples;

        Destroy();

        return true;
    }

    void Texture::RegisterObject()
    {
        RegisterFactory<Texture>();
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


    bool Texture::Create(const ImageLevel* initialData)
    {
        return true;
    }

    void Texture::Destroy()
    {
        SafeDelete(_impl);
    }
}
