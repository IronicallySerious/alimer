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

#include "../Resource/Resource.h"
#include "../Graphics/Backend.h"
#include "../Graphics/GPUResource.h"
#include "../Math/MathUtil.h"

namespace alimer
{
    /// Defines a Texture class.
    class ALIMER_API Texture final : public Resource, public GPUResource
    {
        ALIMER_OBJECT(Texture, Resource);
    protected:
        /// Constructor.
        Texture(GraphicsDevice* device, const TextureDescriptor* descriptor);

    public:
        static const uint32_t MaxPossible = ~0U;

        /// Constructor.
        Texture(GraphicsDevice* device);

        /// Destructor.
        ~Texture() override;

        /// Destroy the backend handle.
        void Destroy() override;

        /// Defines texture from an existing API-handle
        void DefineFromHandle(TextureHandle handle,
            TextureType type, 
            uint32_t width, 
            uint32_t height, 
            uint32_t depth,
            uint32_t arraySize,
            uint32_t mipLevels,
            PixelFormat format,
            TextureUsage usage,
            SampleCount sampleCount);

        /// Get the backend handle.
        TextureHandle GetHandle() const { return _handle; }

        /// Get the type of the texture.
        TextureType GetTextureType() const { return _type; }

        /// Get the texture usage.
        TextureUsage GetUsage() const { return _usage; }

        /// Get the pixel format.
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

        /// Get the array size.
        uint32_t GetArraySize() const { return _arraySize; }

        /// Get the sample count.
        SampleCount GetSamples() const { return _samples; }

    private:
        bool PlatformCreate(const void* pInitData);
        void PlatformDestroy();

    private:
        bool            _externalHandle = false;
        TextureHandle   _handle = BACKEND_INVALID_HANDLE;

        TextureType     _type = TextureType::Type1D;
        uint32_t        _width = 0;
        uint32_t        _height = 0;
        uint32_t        _depth = 0;
        uint32_t        _arraySize = 0;
        uint32_t        _mipLevels = 0;
        SampleCount     _samples = SampleCount::Count1;
        PixelFormat     _format = PixelFormat::Unknown;
        TextureUsage    _usage = TextureUsage::None;
    };
}
