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

#include "Graphics/Framebuffer.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Log.h"

namespace alimer
{
    Framebuffer::Framebuffer(GraphicsDevice* device, const FramebufferDescriptor* descriptor)
        : GPUResource(device, Type::Framebuffer)
    {
        const uint32_t colorAttachmentsCount = Max(MaxColorAttachments, graphics->GetCaps().limits.maxColorAttachments);
        _colorAttachments.reserve(colorAttachmentsCount);
        _width = UINT32_MAX;
        _height = UINT32_MAX;
        _layers = 1;

        for (uint32_t i = 0; i < colorAttachmentsCount; ++i)
        {
            if (descriptor->colorAttachments[i].texture == nullptr)
            {
                continue;
            }

            const uint32_t level = descriptor->colorAttachments[i].level;
            _width = Min(_width, descriptor->colorAttachments[i].texture->GetWidth(level));
            _height = Min(_height, descriptor->colorAttachments[i].texture->GetHeight(level));
            _colorAttachments.push_back(descriptor->colorAttachments[i]);
        }

        if (descriptor->depthStencilAttachment.texture != nullptr)
        {
            const uint32_t level = descriptor->depthStencilAttachment.level;
            _depthStencilAttachment = descriptor->depthStencilAttachment;
            _width = Min(_width, _depthStencilAttachment.texture->GetWidth(level));
            _height = Min(_height, _depthStencilAttachment.texture->GetHeight(level));
        }
    }

    const Texture* Framebuffer::GetColorTexture(uint32_t index) const
    {
        if (index >= _colorAttachments.size())
        {
            ALIMER_LOGERROR("Framebuffer::GetColorTexture: Index is out of range. Requested {} but only {} color slots are available.", index, _colorAttachments.size());
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
