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

#include "../Graphics/Types.h"
#include "../Graphics/PixelFormat.h"
#include "../Resource/Resource.h"

namespace Alimer
{
    class Graphics;

    struct TextureDescription
    {
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

    /// Defines a Texture class.
    class ALIMER_API Texture : public Resource
    {
    protected:
        /// Constructor.
        Texture(Graphics* graphics);

    public:
        /// Destructor.
        virtual ~Texture();

        const TextureDescription &GetDescription() const { return _description; }

        inline TextureType GetTextureType() const { return _description.type; }
        inline TextureUsage GetUsage() const { return _description.usage; }
        inline PixelFormat GetFormat() const { return _description.format; }
        inline uint32_t GetWidth() const { return _description.width; }
        inline uint32_t GetHeight() const { return _description.height; }
        inline uint32_t GetDepth() const { return _description.depth; }
        inline uint32_t GetMipLevels() const { return _description.mipLevels; }
        inline uint32_t GetArrayLayers() const { return _description.arrayLayers; }
        inline SampleCount GetSamples() const { return _description.samples; }

        uint32_t GetLevelWidth(uint32_t mipLevel) const
        {
            return std::max(1u, _description.width >> mipLevel);
        }

        uint32_t GetLevelHeight(uint32_t mipLevel) const
        {
            return std::max(1u, _description.height >> mipLevel);
        }

        uint32_t GetLevelDepth(uint32_t mipLevel) const
        {
            return std::max(1u, _description.depth >> mipLevel);
        }

    protected:
        Graphics* _graphics;
        TextureDescription _description{};

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Texture);
    };
}
