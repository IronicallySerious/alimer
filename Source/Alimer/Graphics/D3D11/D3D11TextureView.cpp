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

#include "D3D11TextureView.h"
#include "D3D11Texture.h"
#include "D3D11GraphicsDevice.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11TextureView::D3D11TextureView(const D3D11Texture* texture, const TextureViewDescriptor* descriptor)
        : TextureView(texture)
    {
        const uint32_t arrayLayers = texture->GetArrayLayers();
        const TextureType textureType = texture->GetTextureType();
        const uint32_t arrayMultiplier = (textureType == TextureType::TypeCube) ? 6 : 1;
        const TextureUsage usage = texture->GetUsage();
        const bool isTextureArray = arrayLayers > 1;
        const bool isTextureMs = static_cast<uint32_t>(texture->GetSamples()) > 1;

        const uint32_t mipLevel = descriptor->baseMipLevel;
        const uint32_t firstArraySlice = descriptor->baseArrayLayer;
        const uint32_t arraySize = descriptor->layerCount;

        if (any(usage & TextureUsage::RenderTarget))
        {
            if (!IsDepthStencilFormat(descriptor->format))
            {
                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.Format = GetDxgiFormat(descriptor->format);
                switch (textureType)
                {
                case TextureType::Type1D:
                    if (isTextureArray)
                    {
                        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
                        rtvDesc.Texture1DArray.MipSlice = mipLevel;
                        rtvDesc.Texture1DArray.FirstArraySlice = firstArraySlice;
                        rtvDesc.Texture1DArray.ArraySize = arraySize;
                    }
                    else
                    {
                        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
                        rtvDesc.Texture1D.MipSlice = mipLevel;
                    }
                    break;
                case TextureType::Type2D:
                case TextureType::TypeCube:
                    if (texture->GetArrayLayers() * arrayMultiplier > 1)
                    {
                        if (isTextureMs)
                        {
                            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                            rtvDesc.Texture2DMSArray.FirstArraySlice = firstArraySlice * arrayMultiplier;
                            rtvDesc.Texture2DMSArray.ArraySize = arraySize * arrayMultiplier;
                        }
                        else
                        {
                            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                            rtvDesc.Texture2DArray.MipSlice = mipLevel;
                            rtvDesc.Texture2DArray.FirstArraySlice = firstArraySlice * arrayMultiplier;
                            rtvDesc.Texture2DArray.ArraySize = arraySize * arrayMultiplier;
                        }
                    }
                    else
                    {
                        if (isTextureMs)
                        {
                            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
                        }
                        else
                        {
                            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                            rtvDesc.Texture2D.MipSlice = mipLevel;
                        }
                    }

                    break;

                case TextureType::Type3D:
                    assert(isTextureArray == false);
                    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
                    break;

                default:
                    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_UNKNOWN;
                    ALIMER_LOGCRITICAL("Invalid texture type");
                    break;
                }

                HRESULT hr = texture->GetD3DDevice()->CreateRenderTargetView(
                    texture->GetResource(),
                    &rtvDesc,
                    _rtv.ReleaseAndGetAddressOf()
                );
                if (FAILED(hr))
                {
                    ALIMER_LOGCRITICAL("[D3D] - CreateRenderTargetView failed");
                }
            }
            else
            {

                D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
                dsvDesc.Format = GetDxgiFormat(descriptor->format);
                switch (textureType)
                {
                case TextureType::Type1D:
                    if (isTextureArray)
                    {
                        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
                        dsvDesc.Texture1DArray.MipSlice = mipLevel;
                        dsvDesc.Texture1DArray.FirstArraySlice = firstArraySlice;
                        dsvDesc.Texture1DArray.ArraySize = arraySize;
                    }
                    else
                    {
                        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
                        dsvDesc.Texture1D.MipSlice = mipLevel;
                    }
                    break;
                case TextureType::Type2D:
                case TextureType::TypeCube:
                    if (arrayLayers * arrayMultiplier > 1)
                    {
                        if (isTextureMs)
                        {
                            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
                            dsvDesc.Texture2DMSArray.FirstArraySlice = firstArraySlice * arrayMultiplier;
                            dsvDesc.Texture2DMSArray.ArraySize = arraySize * arrayMultiplier;
                        }
                        else
                        {
                            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                            dsvDesc.Texture2DArray.MipSlice = mipLevel;
                            dsvDesc.Texture2DArray.FirstArraySlice = firstArraySlice * arrayMultiplier;
                            dsvDesc.Texture2DArray.ArraySize = arraySize * arrayMultiplier;
                        }
                    }
                    else
                    {
                        if (isTextureMs)
                        {
                            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
                        }
                        else
                        {
                            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                            dsvDesc.Texture2D.MipSlice = mipLevel;
                        }
                    }

                    break;

                default:
                    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_UNKNOWN;
                    ALIMER_LOGCRITICAL("Invalid texture type");
                    break;
                }

                HRESULT hr = texture->GetD3DDevice()->CreateDepthStencilView(
                    texture->GetResource(),
                    &dsvDesc,
                    _dsv.ReleaseAndGetAddressOf()
                );
                if (FAILED(hr))
                {
                    ALIMER_LOGCRITICAL("[D3D11] - CreateDepthStencilView failed");
                }
            }
        }
    }

    D3D11TextureView::~D3D11TextureView()
    {

    }
}
