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
#include "../Graphics/GraphicsDevice.h"
#include "../Core/Log.h"

namespace Alimer
{
    Framebuffer::Framebuffer(Graphics* device, const FramebufferDescriptor* descriptor)
        : GraphicsResource(device)
    {
        _width = UINT32_MAX;
        _height = UINT32_MAX;
        _layers = 1;

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const FramebufferAttachment& attachment = descriptor->colorAttachments[i];
            if (attachment.texture == nullptr)
                continue;

            uint32_t mipLevel = attachment.mipLevel;
            Texture* texture = attachment.texture;
            _width = min(_width, texture->GetWidth(mipLevel));
            _height = min(_height, texture->GetHeight(mipLevel));
            _colorAttachments.push_back(attachment);
        }

        if (descriptor->depthStencilAttachment.texture != nullptr)
        {
            uint32_t mipLevel = descriptor->depthStencilAttachment.mipLevel;
            Texture* texture = descriptor->depthStencilAttachment.texture;
            _width = min(_width, texture->GetWidth(mipLevel));
            _height = min(_height, texture->GetHeight(mipLevel));
            _depthStencilAttachment = descriptor->depthStencilAttachment;
        }
    }

    static bool ValidateAttachment(const Texture* texture,
        uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize,
        bool isDepthAttachment)
    {
#ifndef _DEBUG
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

    void Framebuffer::AttachColorTarget(const SharedPtr<Texture>& colorTexture, uint32_t index, uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize)
    {
        if (index >= _colorAttachments.size())
        {
            ALIMER_LOGERRORF("Framebuffer attachment error, requested color index %u need to be in rage 0-%u", index, static_cast<uint32_t>(_colorAttachments.size()));
            return;
        }

        if (ValidateAttachment(
            colorTexture.Get(),
            mipLevel,
            firstArraySlice,
            arraySize,
            false))
        {
            if (colorTexture)
            {
                _width = min(_width, colorTexture->GetWidth(mipLevel));
                _height = min(_height, colorTexture->GetHeight(mipLevel));
            }
            else
            {
            }
        }
    }

    void Framebuffer::AttachDepthStencilTarget(const SharedPtr<Texture>& depthStencil, uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize)
    {
        if (ValidateAttachment(
            depthStencil.Get(),
            mipLevel,
            firstArraySlice,
            arraySize,
            true))
        {
            if (depthStencil)
            {
                _width = min(_width, depthStencil->GetWidth(mipLevel));
                _height = min(_height, depthStencil->GetHeight(mipLevel));
            }
            else
            {
            }
        }
    }

    const Texture* Framebuffer::GetColorTexture(uint32_t index) const
    {
        if (index >= _colorAttachments.size())
        {
            ALIMER_LOGERRORF("Framebuffer::GetColorTexture: Index is out of range. Requested %u but only %u color slots are available.", index, (uint32_t)(_colorAttachments.size()));
            return nullptr;
        }

        return _colorAttachments[index].texture;
    }

    const Texture* Framebuffer::GetDepthStencilTexture() const
    {
        return _depthStencilAttachment.texture;
    }
}
