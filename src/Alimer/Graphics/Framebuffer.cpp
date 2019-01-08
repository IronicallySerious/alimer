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

#include "../Graphics/Framebuffer.h"
#include "../Graphics/GPUDevice.h"
#include "../Graphics/GPUDeviceImpl.h"
#include "../Core/Log.h"

namespace alimer
{
    Framebuffer::Framebuffer()
        : GPUResource(GetSubsystem<GPUDevice>(), Type::Framebuffer)
    {
        _framebuffer = _device->GetImpl()->CreateFramebuffer();
        _colorAttachments.Resize(_device->GetLimits().maxColorAttachments);
        _width = UINT32_MAX;
        _height = UINT32_MAX;
        _layers = 1;
    }

#if defined(ALIMER_DEV)
    static bool ValidateAttachment(Texture* texture, uint32_t level, uint32_t slice, bool isDepthAttachment)
    {
        if (texture == nullptr)
        {
            ALIMER_LOGERROR("Framebuffer attachment error : texture is null.");
            return false;
        }

        if (level >= texture->GetMipLevels())
        {
            ALIMER_LOGERROR("Framebuffer attachment error : mipLevel out of bound.");
            return false;
        }

        /*if (attachment.layerCount != RemainingArrayLayers)
        {
            if (attachment.layerCount == 0)
            {
                ALIMER_LOGERROR("Error when attaching texture to framebuffer : Requested to attach zero array slices");
                return false;
            }

            if (attachment.texture->GetTextureType() == TextureType::Type3D)
            {
                if (attachment.baseArrayLayer + attachment.layerCount > attachment.texture->GetDepth())
                {
                    ALIMER_LOGERROR("Error when attaching texture to framebuffer : Requested depth index is out of bound.");
                    return false;
                }
            }
            else
            {
                if (attachment.baseArrayLayer + attachment.layerCount > attachment.texture->GetArrayLayers())
                {
                    ALIMER_LOGERROR("Error when attaching texture to framebuffer : Requested array index is out of bound.");
                    return false;
                }
            }
        }*/

        if (isDepthAttachment)
        {
            if (IsDepthStencilFormat(texture->GetFormat()) == false)
            {
                ALIMER_LOGERROR("Error when attaching texture to framebuffer : Attaching to depth-stencil target, but resource has color format.");
                return false;
            }

            if (!any(texture->GetUsage() & TextureUsage::OutputAttachment))
            {
                ALIMER_LOGERROR("Error when attaching texture to FBO: Attaching to depth-stencil target, the texture has no OutputAttachment usage flag.");
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

            if (!any(texture->GetUsage() & TextureUsage::OutputAttachment))
            {
                ALIMER_LOGERROR("Error when attaching texture to FBO: Attaching to color target, the texture has no OutputAttachment usage flag.");
                return false;
            }
        }

        return true;
    }
#endif

    void Framebuffer::SetColorAttachment(uint32_t index, Texture* colorTexture, uint32_t level, uint32_t slice)
    {
        if (index >= _colorAttachments.Size())
        {
            ALIMER_LOGERROR("Framebuffer attachment error, requested color index {} need to be in rage 0-{}", index, _colorAttachments.Size());
            return;
        }

        if (ValidateAttachment(colorTexture, level, slice, false))
        {
            if (colorTexture)
            {
                _width = min(_width, colorTexture->GetWidth(level));
                _height = min(_height, colorTexture->GetHeight(level));
                _colorAttachments[index].texture = colorTexture;
                _colorAttachments[index].level = level;
                _colorAttachments[index].slice = slice;
                _framebuffer->SetColorAttachment(index, colorTexture->GetGPUTexture(), level, slice);
            }
            else
            {
                _colorAttachments[index].texture = nullptr;
                _colorAttachments[index].level = 0;
                _colorAttachments[index].slice = 0;
                //_colorAttachments[index].layerCount = RemainingArrayLayers;
                _framebuffer->SetColorAttachment(index, nullptr, level, slice);
            }

            
        }
    }

    void Framebuffer::SetDepthStencilAttachment(Texture* depthStencilTexture, uint32_t level, uint32_t slice)
    {
        if (ValidateAttachment(depthStencilTexture, level, slice, true))
        {
            if (depthStencilTexture)
            {
                _width = min(_width, depthStencilTexture->GetWidth(level));
                _height = min(_height, depthStencilTexture->GetHeight(level));
                _depthStencilAttachment.texture = depthStencilTexture;
                _depthStencilAttachment.level = level;
                _depthStencilAttachment.slice = slice;
                _framebuffer->SetDepthStencilAttachment(depthStencilTexture->GetGPUTexture(), level, slice);
            }
            else
            {
                _depthStencilAttachment.texture = nullptr;
                _depthStencilAttachment.level = 0;
                _depthStencilAttachment.slice = 0;
                _framebuffer->SetDepthStencilAttachment(nullptr, level, slice);
            }
        }
    }

    const Texture* Framebuffer::GetColorTexture(uint32_t index) const
    {
        if (index >= _colorAttachments.Size())
        {
            ALIMER_LOGERROR("Framebuffer::GetColorTexture: Index is out of range. Requested {} but only {} color slots are available.", index, _colorAttachments.Size());
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
