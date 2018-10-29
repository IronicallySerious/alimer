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

#include "D3D11Framebuffer.h"
#include "D3D11GraphicsDevice.h"
#include "D3D11Texture.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"

namespace Alimer
{
    D3D11Framebuffer::D3D11Framebuffer(D3D11GraphicsDevice* device, const FramebufferDescriptor* descriptor)
        : Framebuffer(device, descriptor)
    {
        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const FramebufferAttachment& attachment = descriptor->colorAttachments[i];
            if (attachment.texture == nullptr)
                continue;

            D3D11Texture* texture = static_cast<D3D11Texture*>(attachment.texture);
            _colorRtvs[_viewsCount++] = texture->GetRTV(attachment.mipLevel, attachment.slice);
        }

        if (descriptor->depthStencilAttachment.texture != nullptr)
        {
            const FramebufferAttachment& attachment = descriptor->depthStencilAttachment;
            D3D11Texture* texture = static_cast<D3D11Texture*>(attachment.texture);
            _depthStencilView = texture->GetDSV(attachment.mipLevel, attachment.slice);
        }
    }

    uint32_t D3D11Framebuffer::Bind(ID3D11DeviceContext* context) const
    {
        context->OMSetRenderTargets(_viewsCount, _colorRtvs, _depthStencilView);
        return _viewsCount;
    }
}
