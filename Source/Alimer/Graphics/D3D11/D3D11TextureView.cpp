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
        : TextureView(texture, descriptor)
        , _d3dTexture(texture)
        , _textureUsage(texture->GetUsage())
    {
    }

    D3D11TextureView::~D3D11TextureView()
    {

    }

    ID3D11ShaderResourceView* D3D11TextureView::GetSRV() const
    {
        if (!_srv)
        {
            if (!any(_textureUsage & TextureUsage::ShaderRead))
            {
                ALIMER_LOGWARN("Texture was not created with ShaderRead flag");
                return nullptr;
            }

            uint32_t arrayMultiplier = (_texture->GetTextureType() == TextureType::TypeCube) ? 6 : 1;
            const bool isTextureMs = static_cast<uint32_t>(_texture->GetSamples()) > 1;

            D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
            desc.Format = GetDxgiFormat(_format);
            switch (_texture->GetTextureType())
            {
            case TextureType::Type1D:
                if (_texture->GetArrayLayers() > 1)
                {
                    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.MostDetailedMip = _baseMipLevel;
                    desc.Texture1DArray.FirstArraySlice = _baseArrayLayer;
                    desc.Texture1DArray.ArraySize = _layerCount;
                }
                else
                {
                    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MostDetailedMip = _baseMipLevel;
                    desc.Texture1D.MipLevels = _levelCount;
                }
                break;
            case TextureType::Type2D:
            case TextureType::TypeCube:
                if (_texture->GetArrayLayers() * arrayMultiplier > 1)
                {
                    if (isTextureMs)
                    {
                        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
                        desc.Texture2DMSArray.FirstArraySlice = _baseArrayLayer * arrayMultiplier;
                        desc.Texture2DMSArray.ArraySize = _levelCount * arrayMultiplier;
                    }
                    else
                    {
                        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                        desc.Texture2DArray.MostDetailedMip = _baseMipLevel;
                        desc.Texture2DArray.MipLevels = _levelCount;
                        desc.Texture2DArray.FirstArraySlice = _baseArrayLayer * arrayMultiplier;
                        desc.Texture2DArray.ArraySize = _levelCount * arrayMultiplier;
                    }
                }
                else
                {
                    if (isTextureMs)
                    {
                        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
                    }
                    else
                    {
                        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                        desc.Texture2D.MostDetailedMip = _baseMipLevel;
                        desc.Texture2D.MipLevels = _levelCount;
                    }
                }

                break;

            case TextureType::Type3D:
                assert(_texture->GetArrayLayers() == 1);
                desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
                desc.Texture3D.MostDetailedMip = _baseMipLevel;
                desc.Texture3D.MipLevels = _levelCount;
                break;

            default:
                desc.ViewDimension = D3D11_SRV_DIMENSION_UNKNOWN;
                ALIMER_LOGCRITICAL("Invalid texture type");
                break;
            }

            HRESULT hr = _d3dTexture->GetD3DDevice()->CreateShaderResourceView(
                _d3dTexture->GetResource(),
                &desc,
                _srv.ReleaseAndGetAddressOf()
            );

            if (FAILED(hr))
            {
                ALIMER_LOGCRITICAL("[D3D] - CreateShaderResourceView failed");
            }
        }

        return _srv.Get();
    }

    ID3D11RenderTargetView* D3D11TextureView::GetRTV() const
    {
        if (!_rtv)
        {
            if (!any(_textureUsage & TextureUsage::RenderTarget))
            {
                ALIMER_LOGWARN("Texture was not created with RenderTarget flag");
                return nullptr;
            }

            uint32_t arrayMultiplier = (_texture->GetTextureType() == TextureType::TypeCube) ? 6 : 1;
            const bool isTextureMs = static_cast<uint32_t>(_texture->GetSamples()) > 1;

            D3D11_RENDER_TARGET_VIEW_DESC desc = {};
            desc.Format = GetDxgiFormat(_format);
            switch (_texture->GetTextureType())
            {
            case TextureType::Type1D:
                if (_texture->GetArrayLayers() > 1)
                {
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.MipSlice = _baseMipLevel;
                    desc.Texture1DArray.FirstArraySlice = _baseArrayLayer;
                    desc.Texture1DArray.ArraySize = _levelCount;
                }
                else
                {
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MipSlice = _baseMipLevel;
                }
                break;
            case TextureType::Type2D:
            case TextureType::TypeCube:
                if (_texture->GetArrayLayers() * arrayMultiplier > 1)
                {
                    if (isTextureMs)
                    {
                        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                        desc.Texture2DMSArray.FirstArraySlice = _baseArrayLayer * arrayMultiplier;
                        desc.Texture2DMSArray.ArraySize = _layerCount * arrayMultiplier;
                    }
                    else
                    {
                        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                        desc.Texture2DArray.MipSlice = _baseMipLevel;
                        desc.Texture2DArray.FirstArraySlice = _baseArrayLayer * arrayMultiplier;
                        desc.Texture2DArray.ArraySize = _layerCount * arrayMultiplier;
                    }
                }
                else
                {
                    if (isTextureMs)
                    {
                        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
                    }
                    else
                    {
                        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                        desc.Texture2D.MipSlice = _baseMipLevel;
                    }
                }

                break;

            case TextureType::Type3D:
                assert(_texture->GetArrayLayers() == 1);
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
                break;

            default:
                desc.ViewDimension = D3D11_RTV_DIMENSION_UNKNOWN;
                ALIMER_LOGCRITICAL("Invalid texture type");
                break;
            }

            HRESULT hr = _d3dTexture->GetD3DDevice()->CreateRenderTargetView(
                _d3dTexture->GetResource(),
                &desc,
                _rtv.ReleaseAndGetAddressOf()
            );

            if (FAILED(hr))
            {
                ALIMER_LOGCRITICAL("[D3D] - CreateRenderTargetView failed");
            }
        }

        return _rtv.Get();
    }

    ID3D11DepthStencilView* D3D11TextureView::GetDSV() const
    {
        if (!_dsv)
        {
            if (!any(_textureUsage & TextureUsage::RenderTarget))
            {
                ALIMER_LOGWARN("Texture was not created with RenderTarget flag");
                return nullptr;
            }

            if (!IsDepthStencilFormat(_format))
            {
                ALIMER_LOGWARN("[D3D11] - Cannot create depth stencil view for color pixel format.");
                return nullptr;
            }

            uint32_t arrayMultiplier = (_texture->GetTextureType() == TextureType::TypeCube) ? 6 : 1;
            const bool isTextureMs = static_cast<uint32_t>(_texture->GetSamples()) > 1;

            D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
            desc.Format = GetDxgiFormat(_format);
            switch (_texture->GetTextureType())
            {
            case TextureType::Type1D:
                if (_texture->GetArrayLayers() > 1)
                {
                    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.MipSlice = _baseMipLevel;
                    desc.Texture1DArray.FirstArraySlice = _baseArrayLayer;
                    desc.Texture1DArray.ArraySize = _layerCount;
                }
                else
                {
                    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MipSlice = _baseMipLevel;
                }
                break;
            case TextureType::Type2D:
            case TextureType::TypeCube:
                if (_texture->GetArrayLayers() * arrayMultiplier > 1)
                {
                    if (isTextureMs)
                    {
                        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
                        desc.Texture2DMSArray.FirstArraySlice = _baseArrayLayer * arrayMultiplier;
                        desc.Texture2DMSArray.ArraySize = _layerCount * arrayMultiplier;
                    }
                    else
                    {
                        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                        desc.Texture2DArray.MipSlice = _baseMipLevel;
                        desc.Texture2DArray.FirstArraySlice = _baseArrayLayer * arrayMultiplier;
                        desc.Texture2DArray.ArraySize = _layerCount * arrayMultiplier;
                    }
                }
                else
                {
                    if (isTextureMs)
                    {
                        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
                    }
                    else
                    {
                        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                        desc.Texture2D.MipSlice = _baseMipLevel;
                    }
                }

                break;

            default:
                desc.ViewDimension = D3D11_DSV_DIMENSION_UNKNOWN;
                ALIMER_LOGCRITICAL("Invalid texture type");
                break;
            }

            HRESULT hr = _d3dTexture->GetD3DDevice()->CreateDepthStencilView(
                _d3dTexture->GetResource(),
                &desc,
                _dsv.ReleaseAndGetAddressOf()
            );
            if (FAILED(hr))
            {
                ALIMER_LOGCRITICAL("[D3D11] - CreateDepthStencilView failed");
            }
        }

        return _dsv.Get();
    }
}
