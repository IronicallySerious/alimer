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

#include "FramebufferD3D11.h"
#include "DeviceD3D11.h"
#include "TextureD3D11.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"

namespace Alimer
{
    FramebufferD3D11::FramebufferD3D11(DeviceD3D11* device, uint32_t colorAttachmentsCount, const FramebufferAttachment* colorAttachments, const FramebufferAttachment* depthStencilAttachment)
        : Framebuffer(device, colorAttachmentsCount, colorAttachments, depthStencilAttachment)
    {
        for (uint32_t i = 0; i < colorAttachmentsCount; i++)
        {
            auto d3d11Texture = static_cast<TextureD3D11*>(colorAttachments[i].texture);
            _colorRtvs[i] = d3d11Texture->GetRTV(colorAttachments[i].baseMipLevel, colorAttachments[i].baseArrayLayer, colorAttachments[i].layerCount);
        }

        if (depthStencilAttachment != nullptr)
        {
            auto d3d11Texture = static_cast<TextureD3D11*>(depthStencilAttachment->texture);
            _depthStencilView = d3d11Texture->GetDSV(depthStencilAttachment->baseMipLevel, depthStencilAttachment->baseArrayLayer, depthStencilAttachment->layerCount);
        }
    }

    FramebufferD3D11::~FramebufferD3D11()
    {
        Destroy();
    }

    void FramebufferD3D11::Destroy()
    {
        
    }

    uint32_t FramebufferD3D11::Bind(ID3D11DeviceContext* context) const
    {
        context->OMSetRenderTargets(GetColorAttachmentsCount(), _colorRtvs, _depthStencilView);
        return GetColorAttachmentsCount();
    }
}
