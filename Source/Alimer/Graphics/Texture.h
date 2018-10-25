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

#pragma once

#include "../Math/MathUtil.h"
#include "../Graphics/GraphicsResource.h"
#include "../Resource/Resource.h"
#include "../Resource/Image.h"
#include "../Base/HashMap.h"

namespace Alimer
{
    /// Texture types.
    enum class TextureType : uint32_t
    {
        Type1D = 0,
        Type2D,
        Type3D,
        TypeCube,
    };

    enum class TextureUsage : uint32_t
    {
        Unknown = 0,
        ShaderRead = 1 << 0,
        ShaderWrite = 1 << 1,
        RenderTarget = 1 << 2,
    };
    ALIMER_BITMASK(TextureUsage);

    struct TextureDescriptor
    {
    public:
        TextureType type = TextureType::Type2D;
        TextureUsage usage = TextureUsage::ShaderRead;
        PixelFormat format = PixelFormat::RGBA8UNorm;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
        SampleCount samples = SampleCount::Count1;
    };

    struct TextureViewDescriptor
    {
        PixelFormat format;
        uint32_t baseMipLevel = 0;
        uint32_t levelCount = RemainingMipLevels;
        uint32_t baseArrayLayer = 0;
        uint32_t layerCount = RemainingArrayLayers;
    };

    class TextureView;

    /// Defines a Texture class.
    class ALIMER_API Texture : public GraphicsResource
    {
    protected:
        /// Constructor.
        Texture(GraphicsDevice* device, const TextureDescriptor* descriptor);

    public:
        SharedPtr<TextureView> GetView(uint32_t mipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const;
        SharedPtr<TextureView> GetDefaultTextureView() const;

        TextureType GetTextureType() const { return _type; }
        TextureUsage GetUsage() const { return _usage; }
        PixelFormat GetFormat() const { return _format; }

        /**
        * Get a mipLevel width.
        */
        uint32_t GetWidth(uint32_t mipLevel = 0) const { return (mipLevel < _mipLevels) ? Max(1u, _width >> mipLevel) : 0; }

        /**
        * Get a mipLevel height.
        */
        uint32_t GetHeight(uint32_t mipLevel = 0) const { return (mipLevel < _mipLevels) ? Max(1u, _height >> mipLevel) : 0; }

        /**
        * Get a mipLevel height.
        */
        uint32_t GetDepth(uint32_t mipLevel = 0) const { return (mipLevel < _mipLevels) ? Max(1u, _depth >> mipLevel) : 0; }

        /**
        * Get the number of mip-levels.
        */
        uint32_t GetMipLevels() const { return _mipLevels; }

        /**
        * Get the array layers.
        */
        uint32_t GetArrayLayers() const { return _arrayLayers; }

        /**
        * Get the sample count
        */
        SampleCount GetSamples() const { return _samples; }

    protected:
        void InvalidateViews();

        TextureType _type = TextureType::Type1D;
        TextureUsage _usage = TextureUsage::Unknown;
        PixelFormat _format = PixelFormat::Unknown;
        uint32_t _width = 0;
        uint32_t _height = 0;
        uint32_t _depth = 0;
        uint32_t _mipLevels = 1;
        uint32_t _arrayLayers = 1;
        SampleCount _samples = SampleCount::Count1;

    private:
        virtual SharedPtr<TextureView> CreateTextureView(const TextureViewDescriptor* descriptor) const = 0;
        mutable SharedPtr<TextureView> _defaultTextureView;
        mutable Util::HashMap<SharedPtr<TextureView>> _views;
    };

    class ALIMER_API TextureView : public RefCounted
    {
    protected:
        TextureView(const Texture* resource)
            : _texture(resource)
        {

        }
    public:
        /**
        * Get the texture referenced by the view.
        */
        const Texture* GetTexture() const { return _texture; }

    protected:
        const Texture* _texture;
    };
}
