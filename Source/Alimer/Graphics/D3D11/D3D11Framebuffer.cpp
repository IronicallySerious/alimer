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
    D3D11Framebuffer::D3D11Framebuffer(D3D11GraphicsDevice* device)
        : Framebuffer(device)
    {
        /*for (uint32_t i = 0; i < MaxColorAttachments; ++i)
        {
            const RenderPassAttachment& colorAttachment = descriptor->colorAttachments[i];
            Texture* texture = colorAttachment.texture;
            if (!texture)
                continue;

            D3D11Texture* d3dTexture = static_cast<D3D11Texture*>(texture);

            D3D11_RENDER_TARGET_VIEW_DESC viewDesc = {};
            viewDesc.Format = d3dTexture->GetDXGIFormat();

            const uint32_t arrayLayers = d3dTexture->GetArrayLayers();
            const SampleCount samples = d3dTexture->GetSamples();
            const uint32_t mipSlice = colorAttachment.mipLevel;

            switch (d3dTexture->GetTextureType())
            {
            case TextureType::Type1D:
                if (arrayLayers <= 1)
                {
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = mipSlice;
                }
                else
                {
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MipSlice = mipSlice;
                    viewDesc.Texture1DArray.FirstArraySlice = colorAttachment.slice;
                    viewDesc.Texture1DArray.ArraySize = arrayLayers;
                }

                break;

            case TextureType::Type2D:
                if (arrayLayers <= 1)
                {
                    if (samples <= SampleCount::Count1)
                    {
                        viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                        viewDesc.Texture2D.MipSlice = mipSlice;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
                    }
                }
                else
                {
                    if (samples <= SampleCount::Count1)
                    {
                        viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MipSlice = mipSlice;
                        viewDesc.Texture2DArray.FirstArraySlice = colorAttachment.slice;
                        viewDesc.Texture2DArray.ArraySize = arrayLayers;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                        viewDesc.Texture2DMSArray.FirstArraySlice = colorAttachment.slice;
                        viewDesc.Texture2DMSArray.ArraySize = arrayLayers;
                    }
                }

                break;

            case TextureType::Type3D:
                viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
                viewDesc.Texture3D.MipSlice = mipSlice;
                viewDesc.Texture3D.FirstWSlice = colorAttachment.slice;
                viewDesc.Texture3D.WSize = arrayLayers;
                break;

            case TextureType::TypeCube:
                // TODO.
                viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                break;

            default:
                break;
            }

            ID3D11RenderTargetView* view;
            ThrowIfFailed(
                _d3dDevice->CreateRenderTargetView(d3dTexture->GetResource(), &viewDesc, &view)
            );

            _views.push_back(view);
            _viewsCount++;
        }*/
    }

    D3D11Framebuffer::~D3D11Framebuffer()
    {
        Destroy();
    }

    void D3D11Framebuffer::Destroy()
    {
        for (ID3D11RenderTargetView* view : _views)
        {
            SafeRelease(view, "ID3D11RenderTargetView");
        }

        _views.clear();
    }


    void D3D11Framebuffer::Bind(ID3D11DeviceContext* context)
    {
        context->OMSetRenderTargets(_viewsCount, _views.data(), _depthStencilView);
    }
}
