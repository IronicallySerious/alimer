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
        None = 0,
        ShaderRead = 1 << 0,
        ShaderWrite = 1 << 1,
        RenderTarget = 1 << 2,
    };
    ALIMER_BITMASK(TextureUsage);

    struct TextureDescriptor
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
    class ALIMER_API Texture : public GraphicsResource
    {
        friend class Graphics;
        ALIMER_OBJECT(Texture, GraphicsResource);

    protected:
        /// Constructor.
        Texture(Graphics* graphics);

    public:
        bool Define(const TextureDescriptor* descriptor, const ImageLevel* initialData = nullptr);

        /// Define texture as 2D.
        bool Define2D(
            uint32_t width, uint32_t height, 
            PixelFormat format = PixelFormat::RGBA8UNorm,
            uint32_t mipLevels = 1,
            uint32_t arrayLayers = 1,
            const ImageLevel* initialData = nullptr,
            TextureUsage usage = TextureUsage::ShaderRead,
            SampleCount samples = SampleCount::Count1);

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

    private:
        virtual bool Create(const ImageLevel* initialData) = 0;

        bool BeginLoad(Stream& source) override;
        /// Finish resource loading. Always called from the main thread. Return true if successful.
        bool EndLoad() override;

        /// Register object factory.
        static void RegisterObject();

    protected:
        TextureType _type = TextureType::Type1D;
        TextureUsage _usage = TextureUsage::None;
        PixelFormat _format = PixelFormat::Unknown;
        uint32_t _width = 0;
        uint32_t _height = 0;
        uint32_t _depth = 0;
        uint32_t _mipLevels = 1;
        uint32_t _arrayLayers = 1;
        SampleCount _samples = SampleCount::Count1;
    };
}
