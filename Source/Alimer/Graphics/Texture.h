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
    ALIMER_BITMASK(TextureUsage);

    enum class TextureColorSpace : uint32_t
    {
        Default = 0,
        sRGB = 1
    };

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
        TextureColorSpace colorSpace = TextureColorSpace::Default;
    };

    struct TextureViewDescriptor
    {
    public:
        PixelFormat format = PixelFormat::Undefined;
        uint32_t baseMipLevel = 0;
        uint32_t levelCount = RemainingMipLevels;
        uint32_t baseArrayLayer = 0;
        uint32_t layerCount = RemainingArrayLayers;
    };

    class TextureView;

    /// Defines a Texture class.
    class ALIMER_API Texture : public Resource, public GpuResource
    {
        ALIMER_OBJECT(Texture, Resource);

    public:
        /// Constructor.
        Texture(Graphics* graphics);

        /// Destructor.
        ~Texture() override;

        /// Destroy the texture and views.
        void Destroy() override;

        /// 
        bool Define(const TextureDescriptor* descriptor, const ImageLevel* initialData = nullptr);

        inline TextureType GetTextureType() const { return _type; }
        inline TextureUsage GetUsage() const { return _usage; }
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

        SharedPtr<TextureView> GetDefaultTextureView() const
        {
            return _defaultTextureView;
        }

        SharedPtr<TextureView> CreateTextureView(const TextureViewDescriptor* descriptor);

#if ALIMER_VULKAN
        void SetVkImage(const TextureDescriptor* descriptor, VkImage vkImage, VkImageUsageFlags usage);
        VkImage GetVkImage() const { return _vkImage; }
#endif

    private:
        void InitFromDescriptor(const TextureDescriptor* descriptor);
        bool Create(const ImageLevel* initialData);

        TextureType _type;
        TextureUsage _usage;
        PixelFormat _format;
        uint32_t _width, _height, _depth;
        uint32_t _mipLevels;
        uint32_t _arrayLayers;
        SampleCount _samples;
        TextureColorSpace _colorSpace;
        SharedPtr<TextureView> _defaultTextureView;
        std::vector<SharedPtr<TextureView>> _views;

#if ALIMER_VULKAN
        VkImage _vkImage = VK_NULL_HANDLE;
#endif
    };

    class TextureView final : public RefCounted
    {
        friend class Texture;

    private:
        /// Constructor.
        TextureView(Graphics* graphics, Texture* texture, const TextureViewDescriptor* descriptor);

    public:
        /// Destructor.
        ~TextureView() override;

        /// Destroy the view.
        void Destroy();

        Texture* GetTexture() const { return _texture.Get(); }
        uint64_t GetId() const { return _id; }
        PixelFormat GetFormat() const { return _format; }
        uint32_t GetBaseMipLevel() const { return _baseMipLevel; }

#if ALIMER_VULKAN
        VkImageView GetNativeHandle() const { return _vkImageView; }
#endif

    private:
        void Create();

        /// Graphics subsystem.
        WeakPtr<Graphics> _graphics;
        WeakPtr<Texture> _texture;
        uint64_t _id;
        PixelFormat _format;
        uint32_t _baseMipLevel;

#if ALIMER_VULKAN
        VkImageView _vkImageView = VK_NULL_HANDLE;
#endif
    };
}
