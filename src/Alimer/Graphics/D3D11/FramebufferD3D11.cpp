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

#include "FramebufferD3D11.h"
#include "DeviceD3D11.h"
#include "TextureD3D11.h"
#include "D3D11Convert.h"
#include "../../Core/Log.h"

namespace alimer
{
    FramebufferD3D11::FramebufferD3D11(DeviceD3D11* device, const FramebufferDescriptor* descriptor)
        : Framebuffer(nullptr, descriptor)
    {
        for (uint32_t i = 0; i < MaxColorAttachments; ++i)
        {
            if (descriptor->colorAttachments[i].texture == nullptr)
            {
                continue;
            }

            const FramebufferAttachment& attachment = descriptor->colorAttachments[i];
            const auto textureDescriptor = attachment.texture->GetDescriptor();
            const uint32_t arraySize = textureDescriptor.arraySize - attachment.slice;
            const bool isTextureMs = static_cast<uint32_t>(textureDescriptor.samples) > 1;
            TextureD3D11* d3d11Texture = static_cast<TextureD3D11*>(attachment.texture->GetGPUTexture());

            D3D11_RENDER_TARGET_VIEW_DESC viewDesc = {};
            viewDesc.Format = d3d11Texture->GetDXGIFormat();
            switch (textureDescriptor.type)
            {
            case TextureType::Type1D:
                if (arraySize > 1)
                {
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MipSlice = attachment.level;
                    viewDesc.Texture1DArray.FirstArraySlice = attachment.slice;
                    viewDesc.Texture1DArray.ArraySize = arraySize;
                }
                else
                {
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = attachment.level;
                }
                break;
            case TextureType::Type2D:
                if (arraySize > 1)
                {
                    if (isTextureMs)
                    {
                        viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                        viewDesc.Texture2DMSArray.FirstArraySlice = attachment.slice;
                        viewDesc.Texture2DMSArray.ArraySize = arraySize;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MipSlice = attachment.level;
                        viewDesc.Texture2DArray.FirstArraySlice = attachment.slice;
                        viewDesc.Texture2DArray.ArraySize = arraySize;
                    }
                }
                else
                {
                    if (isTextureMs)
                    {
                        viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                        viewDesc.Texture2D.MipSlice = attachment.level;
                    }
                }

                break;
            case TextureType::TypeCube:
                if (isTextureMs)
                {
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    viewDesc.Texture2DMSArray.FirstArraySlice = attachment.slice * 6;
                    viewDesc.Texture2DMSArray.ArraySize = arraySize * 6;
                }
                else
                {
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.MipSlice = attachment.level;
                    viewDesc.Texture2DArray.FirstArraySlice = attachment.slice * 6;
                    viewDesc.Texture2DArray.ArraySize = arraySize * 6;
                }
                break;


            case TextureType::Type3D:
                assert(arraySize == 1);
                viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
                viewDesc.Texture3D.MipSlice = attachment.level;
                viewDesc.Texture3D.FirstWSlice = attachment.slice;
                viewDesc.Texture3D.WSize = textureDescriptor.depth;
                break;

            default:
                viewDesc.ViewDimension = D3D11_RTV_DIMENSION_UNKNOWN;
                ALIMER_LOGCRITICAL("Invalid texture type");
                break;
            }

            HRESULT hr = device->GetD3DDevice()->CreateRenderTargetView(
                d3d11Texture->GetResource(), 
                &viewDesc, 
                &_colorRtvs[_colorAttachmentsCount]);
            if (FAILED(hr))
            {
                ALIMER_LOGCRITICAL("[D3D] - CreateRenderTargetView failed");
            }
            _colorAttachmentsCount++;
        }

        if (descriptor->depthStencilAttachment.texture != nullptr)
        {
            const FramebufferAttachment& attachment = descriptor->depthStencilAttachment;
            const auto textureDescriptor = attachment.texture->GetDescriptor();
            const uint32_t arraySize = textureDescriptor.arraySize - attachment.slice;
            const bool isTextureMs = static_cast<uint32_t>(textureDescriptor.samples) > 1;
            TextureD3D11* d3d11Texture = static_cast<TextureD3D11*>(attachment.texture->GetGPUTexture());

            D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
            viewDesc.Format = d3d11Texture->GetDXGIFormat();
            switch (textureDescriptor.type)
            {
            case TextureType::Type1D:
                if (arraySize > 1)
                {
                    viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MipSlice = attachment.level;
                    viewDesc.Texture1DArray.FirstArraySlice = attachment.slice;
                    viewDesc.Texture1DArray.ArraySize = arraySize;
                }
                else
                {
                    viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = attachment.level;
                }
                break;
            case TextureType::Type2D:
                if (arraySize > 1)
                {
                    if (isTextureMs)
                    {
                        viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
                        viewDesc.Texture2DMSArray.FirstArraySlice = attachment.slice;
                        viewDesc.Texture2DMSArray.ArraySize = arraySize;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MipSlice = attachment.level;
                        viewDesc.Texture2DArray.FirstArraySlice = attachment.slice;
                        viewDesc.Texture2DArray.ArraySize = arraySize;
                    }
                }
                else
                {
                    if (isTextureMs)
                    {
                        viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                        viewDesc.Texture2D.MipSlice = attachment.level;
                    }
                }

                break;

            case TextureType::TypeCube:
                if (isTextureMs)
                {
                    viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    viewDesc.Texture2DMSArray.FirstArraySlice = attachment.slice * 6;
                    viewDesc.Texture2DMSArray.ArraySize = arraySize * 6;
                }
                else
                {
                    viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.MipSlice = attachment.level * 6;
                    viewDesc.Texture2DArray.FirstArraySlice = attachment.slice * 6;
                    viewDesc.Texture2DArray.ArraySize = arraySize * 6;
                }
                break;

            case TextureType::Type3D:
                viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                viewDesc.Texture2DArray.MipSlice = attachment.level;
                viewDesc.Texture2DArray.FirstArraySlice = attachment.slice;
                viewDesc.Texture2DArray.ArraySize = textureDescriptor.depth;
                break;

            default:
                viewDesc.ViewDimension = D3D11_DSV_DIMENSION_UNKNOWN;
                ALIMER_LOGCRITICAL("Invalid texture type");
                break;
            }

            HRESULT hr = device->GetD3DDevice()->CreateDepthStencilView(d3d11Texture->GetResource(), &viewDesc, &_depthStencilView);
            if (FAILED(hr))
            {
                ALIMER_LOGCRITICAL("[D3D11] - CreateDepthStencilView failed");
            }
        }
    }

    FramebufferD3D11::~FramebufferD3D11()
    {
        Destroy();
    }

    void FramebufferD3D11::Destroy()
    {
        for (uint32_t i = 0; i < _colorAttachmentsCount; i++)
        {
            ULONG count = _colorRtvs[i]->Release();
            ALIMER_ASSERT(count == 0);
            _colorRtvs[i] = nullptr;
        }

        if (_depthStencilView)
        {
            ULONG count = _depthStencilView->Release();
            ALIMER_ASSERT(count == 0);
            _depthStencilView = nullptr;
        }
    }

    uint32_t FramebufferD3D11::Bind(ID3D11DeviceContext* context) const
    {
        context->OMSetRenderTargets(_colorAttachmentsCount, _colorRtvs, _depthStencilView);
        return _colorAttachmentsCount;
    }
}
