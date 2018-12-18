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
#include "../Graphics/GPUDevice.h"
#include "../Core/Log.h"

namespace Alimer
{
    Framebuffer::Framebuffer(GPUDevice* device, uint32_t colorAttachmentsCount, const FramebufferAttachment* colorAttachments, const FramebufferAttachment* depthStencilAttachment)
        : GPUResource(device, Type::Framebuffer)
    {
        _colorAttachments.Reserve(colorAttachmentsCount);
        _width = UINT32_MAX;
        _height = UINT32_MAX;
        _layers = 1;

        for (uint32_t i = 0; i < colorAttachmentsCount; i++)
        {
            const FramebufferAttachment& attachment = colorAttachments[i];
            Texture* texture = attachment.texture;
            _width = min(_width, texture->GetWidth(attachment.level));
            _height = min(_height, texture->GetHeight(attachment.level));
            _colorAttachments.Push(attachment);
        }

        if (depthStencilAttachment != nullptr)
        {
            uint32_t mipLevel = depthStencilAttachment->level;
            _width = min(_width, depthStencilAttachment->texture->GetWidth(mipLevel));
            _height = min(_height, depthStencilAttachment->texture->GetHeight(mipLevel));
            memcpy(&_depthStencilAttachment, depthStencilAttachment, sizeof(FramebufferAttachment));
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
