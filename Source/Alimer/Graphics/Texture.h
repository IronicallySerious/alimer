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
#include "../Graphics/GPUResource.h"
#include "../Resource/Image.h"
#include "../Base/HashMap.h"

namespace Alimer
{
    /// Defines a Texture class.
    class ALIMER_API Texture : public GPUResource, public Resource
    {
        friend class GPUDevice;
        ALIMER_OBJECT(Texture, Resource);

    protected:
        /// Constructor.
        Texture(GPUDevice* device, 
            TextureType type, uint32_t width, uint32_t height, 
            uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers,
            PixelFormat format, TextureUsage usage, SampleCount samples);

    public:
        TextureType GetTextureType() const { return _type; }
        TextureUsage GetUsage() const { return _usage; }
        PixelFormat GetFormat() const { return _format; }

        /// Get a mipLevel width.
        uint32_t GetWidth(uint32_t mipLevel = 0) const 
        {
            return Max(1u, _width >> mipLevel);
        }

        /// Get a mipLevel height.
        uint32_t GetHeight(uint32_t mipLevel = 0) const 
        {
            return Max(1u, _height >> mipLevel);
        }

        /// Get a mipLevel height.
        uint32_t GetDepth(uint32_t mipLevel = 0) const
        {
            return Max(1u, _depth >> mipLevel);
        }

        /// Get the number of mip-levels.
        uint32_t GetMipLevels() const { return _mipLevels; }

        /// Get the array layers.
        uint32_t GetArrayLayers() const { return _arrayLayers; }

        /// Get the sample count
        SampleCount GetSamples() const { return _samples; }

    private:
        bool BeginLoad(Stream& source) override;
        /// Finish resource loading. Always called from the main thread. Return true if successful.
        bool EndLoad() override;

        /// Register object factory.
        static void RegisterObject();

        TextureType _type = TextureType::Type2D;
        uint32_t _width = 1;
        uint32_t _height = 1;
        uint32_t _depth = 1;
        uint32_t _mipLevels = 1;
        uint32_t _arrayLayers = 1;
        PixelFormat _format = PixelFormat::RGBA8UNorm;
        TextureUsage _usage = TextureUsage::ShaderRead;
        SampleCount _samples = SampleCount::Count1;
    };
}
