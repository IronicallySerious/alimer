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
#include "../Graphics/GraphicsDevice.h"
#include "../Core/Log.h"

namespace Alimer
{
    Texture::Texture(Graphics* graphics)
        : GpuResource(graphics, GpuResourceType::Texture)
    {
    }

    Texture::~Texture()
    {
        _views.clear();
        Destroy();
    }

    void Texture::InitFromDescriptor(const TextureDescriptor* descriptor)
    {
        _type = descriptor->type;
        _usage = descriptor->usage;
        _format = descriptor->format;
        _width = descriptor->width;
        _height = descriptor->height;
        _depth = descriptor->depth;
        _mipLevels = descriptor->depth;
        _arrayLayers = descriptor->arrayLayers;
        _samples = descriptor->samples;
        _colorSpace = descriptor->colorSpace;
    }

    bool Texture::Define(const TextureDescriptor* descriptor, const ImageLevel* initialData)
    {
        _views.clear();
        Destroy();

        // Copy settings.
        InitFromDescriptor(descriptor);

        if (_graphics
            && _graphics->IsInitialized())
        {
            if (Create(initialData))
            {
                return true;
            }
        }

        return false;
    }

    SharedPtr<TextureView> Texture::CreateTextureView(const TextureViewDescriptor* descriptor)
    {
        SharedPtr<TextureView> newView(new TextureView(_graphics, this, descriptor));
        _views.push_back(newView);
        return newView;
    }

    // TextureView
    TextureView::TextureView(Graphics* graphics, Texture* texture, const TextureViewDescriptor* descriptor)
        : _graphics(graphics)
        , _texture(texture)
        , _id(graphics->GetNextUniqueId())
        , _format(descriptor->format)
        , _baseMipLevel(descriptor->baseMipLevel)
    {
        Create();
    }

    TextureView::~TextureView()
    {
        Destroy();
    }
}
