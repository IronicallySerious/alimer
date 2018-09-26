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

#include "../Core/Flags.h"
#include "../Math/MathUtil.h"
#include "../Graphics/GpuResource.h"
#include "../Resource/Resource.h"
#include "../Resource/Image.h"

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

    enum class TextureColorSpace : uint32_t
    {
        Default = 0,
        sRGB = 1
    };

    using TextureUsageFlags = Flags<TextureUsage, uint32_t>;
    ALIMER_FORCE_INLINE TextureUsageFlags operator|(TextureUsage bit0, TextureUsage bit1)
    {
        return TextureUsageFlags(bit0) | bit1;
    }

    ALIMER_FORCE_INLINE TextureUsageFlags operator~(TextureUsage bits)
    {
        return ~(TextureUsageFlags(bits));
    }

    struct TextureDescriptor
    {
        TextureType type = TextureType::Type2D;
        TextureUsageFlags usage = TextureUsage::ShaderRead;
        PixelFormat format = PixelFormat::RGBA8UNorm;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
        SampleCount samples = SampleCount::Count1;
        TextureColorSpace colorSpace = TextureColorSpace::Default;
    };

    /// Defines a Texture class.
    class ALIMER_API Texture : public Resource, public GpuResource
    {
        ALIMER_OBJECT(Texture, Resource);

    protected:
        /// Constructor.
        Texture(GraphicsDevice* device, const TextureDescriptor* descriptor);

    public:
        /// Destructor.
        virtual ~Texture() = default;

        inline TextureType GetTextureType() const { return _type; }
        inline TextureUsageFlags GetUsage() const { return _usage; }
        inline PixelFormat GetFormat() const { return _format; }
        inline uint32_t GetWidth() const { return _width; }
        inline uint32_t GetHeight() const { return _height; }
        inline uint32_t GetDepth() const { return _depth; }
        inline uint32_t GetMipLevels() const { return _mipLevels; }
        inline uint32_t GetArrayLayers() const { return _arrayLayers; }
        inline SampleCount GetSamples() const { return _samples; }

        uint32_t GetLevelWidth(uint32_t mipLevel) const
        {
            return Max(1u, _width >> mipLevel);
        }

        uint32_t GetLevelHeight(uint32_t mipLevel) const
        {
            return Max(1u, _height >> mipLevel);
        }

        uint32_t GetLevelDepth(uint32_t mipLevel) const
        {
            return Max(1u, _depth >> mipLevel);
        }

    protected:
        TextureType _type;
        TextureUsageFlags _usage;
        PixelFormat _format;
        uint32_t _width, _height, _depth;
        uint32_t _mipLevels;
        uint32_t _arrayLayers;
        SampleCount _samples;
        TextureColorSpace _colorSpace;
    };
}
