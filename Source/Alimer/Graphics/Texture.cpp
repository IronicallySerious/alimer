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
    Texture::Texture(GraphicsDevice* device, const TextureDescriptor* descriptor)
        : GraphicsResource(device)
        , _type(descriptor->type)
    {
        _usage = descriptor->usage;
        _format = descriptor->format;
        _width = descriptor->width;
        _height = descriptor->height;
        _depth = descriptor->depth;
        _mipLevels = descriptor->depth;
        _arrayLayers = descriptor->arrayLayers;
        _samples = descriptor->samples;
    }

    void Texture::InvalidateViews()
    {
        _views.clear();
    }

    SharedPtr<TextureView> Texture::GetView(uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const
    {
        if (baseMipLevel >= _mipLevels)
        {
            baseMipLevel = _mipLevels - 1;
        }

        if (baseArrayLayer >= _arrayLayers)
        {
            baseArrayLayer = _arrayLayers - 1;
        }

        if (levelCount == RemainingMipLevels)
        {
            levelCount = _mipLevels - baseMipLevel;
        }
        else if (levelCount + baseMipLevel > _mipLevels)
        {
            levelCount = _mipLevels - baseMipLevel;
        }

        if (layerCount == RemainingArrayLayers)
        {
            layerCount = _arrayLayers - baseArrayLayer;
        }
        else if (layerCount + baseArrayLayer > _arrayLayers)
        {
            layerCount = _arrayLayers - baseArrayLayer;
        }

        Util::Hasher hasher;
        hasher.u32(baseMipLevel);
        hasher.u32(levelCount);
        hasher.u32(baseArrayLayer);
        hasher.u32(layerCount);
        auto hash = hasher.get();

        auto it = _views.find(hash);
        if (it != end(_views))
        {
            return it->second;
        }

        TextureViewDescriptor viewDescriptor;
        viewDescriptor.format = _format;
        viewDescriptor.baseMipLevel = baseMipLevel;
        viewDescriptor.levelCount = levelCount;
        viewDescriptor.baseArrayLayer = baseArrayLayer;
        viewDescriptor.layerCount = layerCount;
        _views[hash] = CreateTextureView(&viewDescriptor);

        return _views[hash];
    }

    SharedPtr<TextureView> Texture::GetDefaultTextureView() const
    {
        if (_defaultTextureView.IsNull())
        {
            _defaultTextureView = GetView(0, _mipLevels, 0, _arrayLayers);
        }

        return _defaultTextureView;
    }
}
