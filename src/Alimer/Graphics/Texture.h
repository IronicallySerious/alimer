//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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

namespace alimer
{
    struct GPUTexture;

    /// Defines a Texture class.
    class ALIMER_API Texture : public GPUResource
    {
        friend class GPUDevice;

    protected:
        /// Constructor.
        Texture();

        /// Constructor.
        Texture(GPUTexture* texture);

    public:
        /// Desturctor
        ~Texture() override;

        void Destroy() override;

        /// Get the type of the texture.
        TextureType GetTextureType() const { return _descriptor.type; }

        /// Get the texture usage.
        TextureUsage GetUsage() const { return _descriptor.usage; }

        /// Get the pixel format.
        PixelFormat GetFormat() const { return _descriptor.format; }

        /// Get a mipLevel width.
        uint32_t GetWidth(uint32_t mipLevel = 0) const 
        {
            return Max(1u, _descriptor.width >> mipLevel);
        }

        /// Get a mipLevel height.
        uint32_t GetHeight(uint32_t mipLevel = 0) const 
        {
            return Max(1u, _descriptor.height >> mipLevel);
        }

        /// Get a mipLevel height.
        uint32_t GetDepth(uint32_t mipLevel = 0) const
        {
            return Max(1u, _descriptor.depth >> mipLevel);
        }

        /// Get the number of mip-levels.
        uint32_t GetMipLevels() const { return _descriptor.mipLevels; }

        /// Get the array size.
        uint32_t GetArraySize() const { return _descriptor.arraySize; }

        /// Get the sample count.
        SampleCount GetSamples() const { return _descriptor.samples; }

        /// Get the texture description.
        const TextureDescriptor& GetDescriptor() const { return _descriptor; }

        /// Get the GPUTexture.
        GPUTexture* GetGPUTexture() const { return _texture; }

    protected:
        bool CreateGPUTexture(const void* initialData);

        GPUTexture* _texture = nullptr;
        TextureDescriptor _descriptor{};
    };

    class Texture2D final : public Texture
    {
    public:
        /// Constructor.
        Texture2D();

        /// Constructor with external texture.
        Texture2D(GPUTexture* texture);

        bool Define(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arraySize, PixelFormat format, TextureUsage usage, SampleCount samples = SampleCount::Count1, const void* initialData = nullptr);
    };
}
