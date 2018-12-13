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

#include "../Graphics/Framebuffer.h"
#include "../Graphics/Graphics.h"
#include "../Debug/Log.h"

namespace Alimer
{
    Framebuffer::Framebuffer(Graphics* graphics)
        : GraphicsResource(graphics)
    {
        _colorAttachments.Resize(graphics->GetFeatures().MaxColorAttachments());
        _width = UINT32_MAX;
        _height = UINT32_MAX;
        _layers = 1;
    }

    bool Framebuffer::Define(const FramebufferDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);

        _width = UINT32_MAX;
        _height = UINT32_MAX;
        _layers = 1;

        Destroy();

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const FramebufferAttachment& attachment = descriptor->colorAttachments[i];
            if (attachment.texture == nullptr)
                continue;

            uint32_t mipLevel = attachment.baseMipLevel;
            Texture* texture = attachment.texture;
            _width = min(_width, texture->GetWidth(mipLevel));
            _height = min(_height, texture->GetHeight(mipLevel));
            _colorAttachments.Push(attachment);
        }

        if (descriptor->depthStencilAttachment.texture != nullptr)
        {
            uint32_t mipLevel = descriptor->depthStencilAttachment.baseMipLevel;
            Texture* texture = descriptor->depthStencilAttachment.texture;
            _width = min(_width, texture->GetWidth(mipLevel));
            _height = min(_height, texture->GetHeight(mipLevel));
            _depthStencilAttachment = descriptor->depthStencilAttachment;
        }

        return Create();
    }

    bool Framebuffer::Create()
    {
        return false;
    }

    static bool ValidateAttachment(const Texture* texture,
        uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize,
        bool isDepthAttachment)
    {
#if !defined(ALIMER_DEV)
        return true;
#endif
        if (texture == nullptr)
        {
            return true;
        }

        if (mipLevel >= texture->GetMipLevels())
        {
            ALIMER_LOGERROR("Framebuffer attachment error : mipLevel out of bound.");
            return false;
        }

        if (arraySize != RemainingArrayLayers)
        {
            if (arraySize == 0)
            {
                ALIMER_LOGERROR("Error when attaching texture to framebuffer : Requested to attach zero array slices");
                return false;
            }

            if (texture->GetTextureType() == TextureType::Type3D)
            {
                if (arraySize + firstArraySlice > texture->GetDepth())
                {
                    ALIMER_LOGERROR("Error when attaching texture to framebuffer : Requested depth index is out of bound.");
                    return false;
                }
            }
            else
            {
                if (arraySize + firstArraySlice > texture->GetArrayLayers())
                {
                    ALIMER_LOGERROR("Error when attaching texture to framebuffer : Requested array index is out of bound.");
                    return false;
                }
            }
        }

        if (isDepthAttachment)
        {
            if (IsDepthStencilFormat(texture->GetFormat()) == false)
            {
                ALIMER_LOGERROR("Error when attaching texture to framebuffer : Attaching to depth-stencil target, but resource has color format.");
                return false;
            }

            if (!any(texture->GetUsage() & TextureUsage::RenderTarget))
            {
                ALIMER_LOGERROR("Error when attaching texture to FBO: Attaching to depth-stencil target, the texture has no RenderTarget usage flag.");
                return false;
            }
        }
        else
        {
            if (IsDepthStencilFormat(texture->GetFormat()))
            {
                ALIMER_LOGERROR("Error when attaching texture to FBO: Attaching to color target, but resource has depth-stencil format.");
                return false;
            }

            if (!any(texture->GetUsage() & TextureUsage::RenderTarget))
            {
                ALIMER_LOGERROR("Error when attaching texture to FBO: Attaching to color target, the texture has no RenderTarget usage flag.");
                return false;
            }
        }

        return true;
    }

    void Framebuffer::SetColorAttachment(uint32_t index, Texture* colorTexture, uint32_t baseMipLevel, uint32_t baseArrayLayer, uint32_t layerCount)
    {
        if (index >= _colorAttachments.Size())
        {
            ALIMER_LOGERRORF("Framebuffer attachment error, requested color index %u need to be in rage 0-%u", index, _colorAttachments.Size());
            return;
        }

        if (ValidateAttachment(colorTexture, baseMipLevel, baseArrayLayer, layerCount, false))
        {
            if (colorTexture)
            {
                _width = min(_width, colorTexture->GetWidth(baseMipLevel));
                _height = min(_height, colorTexture->GetHeight(baseMipLevel));
                _colorAttachments[index].texture = colorTexture;
                _colorAttachments[index].baseMipLevel = baseMipLevel;
                _colorAttachments[index].baseArrayLayer = baseArrayLayer;
                _colorAttachments[index].layerCount = layerCount;
            }
            else
            {
                _colorAttachments[index].texture = nullptr;
                _colorAttachments[index].baseMipLevel = 0;
                _colorAttachments[index].baseArrayLayer = 0;
                _colorAttachments[index].layerCount = RemainingArrayLayers;
            }
            ApplyColorAttachment(index);
        }
    }

    void Framebuffer::SetDepthStencilAttachment(Texture* depthStencilTexture, uint32_t baseMipLevel, uint32_t baseArrayLayer, uint32_t layerCount)
    {
        if (ValidateAttachment(depthStencilTexture, baseMipLevel, baseArrayLayer, layerCount, true))
        {
            if (depthStencilTexture)
            {
                _width = min(_width, depthStencilTexture->GetWidth(baseMipLevel));
                _height = min(_height, depthStencilTexture->GetHeight(baseMipLevel));
                _depthStencilAttachment.texture = depthStencilTexture;
                _depthStencilAttachment.baseMipLevel = baseMipLevel;
                _depthStencilAttachment.baseArrayLayer = baseArrayLayer;
                _depthStencilAttachment.layerCount = layerCount;
            }
            else
            {
                _depthStencilAttachment.texture = nullptr;
                _depthStencilAttachment.baseMipLevel = 0;
                _depthStencilAttachment.baseArrayLayer = 0;
                _depthStencilAttachment.layerCount = RemainingArrayLayers;
            }

            ApplyDepthStencilAttachment();
        }
    }

    const Texture* Framebuffer::GetColorTexture(uint32_t index) const
    {
        if (index >= _colorAttachments.Size())
        {
            ALIMER_LOGERRORF("Framebuffer::GetColorTexture: Index is out of range. Requested %u but only %u color slots are available.", index, _colorAttachments.Size());
            return nullptr;
        }

        return _colorAttachments[index].texture;
    }

    bool Framebuffer::HasDepthStencilAttachment() const
    {
        return _depthStencilAttachment.texture != nullptr;
    }

    const Texture* Framebuffer::GetDepthStencilTexture() const
    {
        return _depthStencilAttachment.texture;
    }
}
