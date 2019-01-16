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
#include "../Resource/Resource.h"
#include "../Base/HashMap.h"

namespace alimer
{
    class GPUTexture;

    /// Defines a Texture class.
    class ALIMER_API Texture final: public Resource
    {
        friend class GPUDevice;
        ALIMER_OBJECT(Texture, Resource);

    public:
        static constexpr uint32_t MaxPossible = ~0U;

        /// Constructor.
        Texture();

        /// Destructor
        virtual ~Texture() override;

        /** 
        * Defines a 2D texture.
        *
        * @param width The width of the texture.
        * @param height The height of the texture.
        * @param format The format of the texture.
        * @param arraySize The array size of the texture.
        * @param mipLevels if equal to MaxPossible then an entire mip chain will be generated from mip level 0. If any other value is given then the data for at least that number of miplevels must be provided.
        * @param pInitData If different than nullptr, pointer to a buffer containing data to initialize the texture with.
        * @param usage The requested usage for the resource.
        * @return A pointer to a new texture, or nullptr if creation failed.
        */
        bool Define2D(uint32_t width, uint32_t height, PixelFormat format, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible, const void* pInitData = nullptr, TextureUsage usage = TextureUsage::ShaderRead);

        /**
        * Defines a 2D multi-sampled texture.
        *
        * @param width The width of the texture.
        * @param height The height of the texture.
        * @param format The format of the texture.
        * @param arraySize The array size of the texture.
        * @param mipLevels if equal to MaxPossible then an entire mip chain will be generated from mip level 0. If any other value is given then the data for at least that number of miplevels must be provided.
        * @param usage The requested bind flags for the resource
        * @param pInitData If different than nullptr, pointer to a buffer containing data to initialize the texture with.
        * @return A pointer to a new texture, or nullptr if creation failed
        */
        bool Define2DMultisample(uint32_t width, uint32_t height, PixelFormat format, SampleCount samples, uint32_t arraySize = 1, TextureUsage usage = TextureUsage::ShaderRead, const void* pInitData = nullptr);

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

        /// Return the device used for creation.
        GPUDevice* GetDevice() const { return _device; }

        /// Get the GPUTexture.
        GPUTexture* GetGPUTexture() const { return _texture; }

    private:
        /// Register object factory.
        static void RegisterObject();

        bool CreateGPUTexture(void* nativeTexture, const void* initialData);
        void Destroy();

    protected:
        GPUDevice* _device;
        GPUTexture* _texture = nullptr;
        uint32_t        _width = 0;
        uint32_t        _height = 0;
        uint32_t        _depth = 0;
        uint32_t        _arraySize = 0;
        uint32_t        _mipLevels = 0;
        SampleCount     _samples = SampleCount::Count1;
        TextureType     _type = TextureType::Type1D;
        PixelFormat     _format = PixelFormat::Unknown;
        TextureUsage    _usage = TextureUsage::None;
    };
}
