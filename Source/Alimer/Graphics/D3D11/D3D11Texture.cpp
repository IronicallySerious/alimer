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

#include "D3D11Texture.h"
#include "D3D11Graphics.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"

namespace Alimer
{
    D3D11Texture::D3D11Texture(D3D11Graphics* graphics, const TextureDescription* pDescription, const ImageLevel* initialData)
        : Texture(graphics, *pDescription)
    {
        // Setup initial data.
        std::vector<D3D11_SUBRESOURCE_DATA> subResourceData;
        if (initialData)
        {
            subResourceData.resize(pDescription->arrayLayers * pDescription->mipLevels);
            for (uint32_t i = 0; i < pDescription->arrayLayers * pDescription->mipLevels; ++i)
            {
                uint32_t rowPitch;
                if (!initialData[i].rowPitch)
                {
                    const uint32_t mipWidth = ((pDescription->width >> i) > 0) ? pDescription->width >> i : 1;
                    const uint32_t mipHeight = ((pDescription->height >> i) > 0) ? pDescription->height >> i : 1;

                    uint32_t rows;
                    CalculateDataSize(
                        mipWidth,
                        mipHeight,
                        pDescription->format,
                        &rows,
                        &rowPitch);
                }
                else
                {
                    rowPitch = initialData[i].rowPitch;
                }

                subResourceData[i].pSysMem = initialData[i].data;
                subResourceData[i].SysMemPitch = rowPitch;
                subResourceData[i].SysMemSlicePitch = 0;
            }
        }

        const bool srgb = pDescription->colorSpace == TextureColorSpace::sRGB;
        _dxgiFormat = d3d::Convert(pDescription->format, srgb);

        switch (pDescription->type)
        {
        case TextureType::Type1D:
        {
            D3D11_TEXTURE1D_DESC d3d11Desc = {};
        }
        break;

        case TextureType::Type2D:
        case TextureType::TypeCube:
        {
            D3D11_TEXTURE2D_DESC d3d11Desc = {};
            d3d11Desc.Width = pDescription->width;
            d3d11Desc.Height = pDescription->height;
            d3d11Desc.MipLevels = pDescription->mipLevels;
            d3d11Desc.ArraySize = pDescription->arrayLayers;
            d3d11Desc.Format = _dxgiFormat;
            d3d11Desc.SampleDesc.Count = static_cast<uint32_t>(pDescription->samples);
            d3d11Desc.Usage = D3D11_USAGE_DEFAULT;

            if (pDescription->usage & TextureUsage::ShaderRead)
            {
                d3d11Desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }

            if (pDescription->usage & TextureUsage::ShaderWrite)
            {
                d3d11Desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
            }

            if (pDescription->usage & TextureUsage::RenderTarget)
            {
                if (!IsDepthStencilFormat(pDescription->format))
                {
                    d3d11Desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
                }
                else
                {
                    d3d11Desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
                }
            }

            const bool dynamic = false;
            d3d11Desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
            d3d11Desc.MiscFlags = 0;
            if (pDescription->type == TextureType::TypeCube)
            {
                d3d11Desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
            }

            graphics->GetD3DDevice()->CreateTexture2D(
                &d3d11Desc,
                subResourceData.data(),
                &_texture2D);

        }
        break;

        default:
            break;
        }

        // Create default shader resource view
        if (pDescription->usage & TextureUsage::ShaderRead)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc = {};
            resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            resourceViewDesc.Format = _dxgiFormat;

            if (FAILED(graphics->GetD3DDevice()->CreateShaderResourceView(
                _resource,
                nullptr,
                &_shaderResourceView)))
            {
                ALIMER_LOGERROR("D3D11 - Failed to create ShaderResourceView for texture.");
            }

            D3D11_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.MaxAnisotropy = (graphics->GetFeatureLevel() > D3D_FEATURE_LEVEL_9_1) ? D3D11_MAX_MAXANISOTROPY : 2;
            samplerDesc.MaxLOD = FLT_MAX;
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

            ThrowIfFailed(graphics->GetD3DDevice()->CreateSamplerState(
                &samplerDesc,
                &_samplerState)
            );
        }
    }

    D3D11Texture::D3D11Texture(D3D11Graphics* graphics, ID3D11Texture2D* nativeTexture)
        : Texture(graphics)
        , _d3dDevice(graphics->GetD3DDevice())
        , _texture2D(nativeTexture)
    {
        D3D11_TEXTURE2D_DESC desc;
        nativeTexture->GetDesc(&desc);

        _description.type = TextureType::Type2D;
        _dxgiFormat = desc.Format;
        _description.format = d3d::Convert(_dxgiFormat);

        _description.width = desc.Width;
        _description.height = desc.Height;
        _description.depth = 1;
        if (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
        {
            _description.type = TextureType::TypeCube;
            _description.arrayLayers = desc.ArraySize / 6;
        }
        else
        {
            _description.arrayLayers = desc.ArraySize;
        }
        _description.mipLevels = desc.MipLevels;

        _description.usage = TextureUsage::Unknown;
        if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
        {
            _description.usage |= TextureUsage::ShaderRead;
        }

        if (desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
        {
            _description.usage |= TextureUsage::ShaderWrite;
        }

        if (desc.BindFlags & D3D11_BIND_RENDER_TARGET
            || desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
        {
            _description.usage |= TextureUsage::RenderTarget;
        }
    }

    D3D11Texture::~D3D11Texture()
    {
        Destroy();
    }

    void D3D11Texture::Destroy()
    {
        if (_shaderResourceView)
        {
#if defined(_DEBUG)
            ULONG refCount = GetRefCount(_shaderResourceView);
            ALIMER_ASSERT_MSG(refCount == 1, "ID3D11ShaderResourceView leakage");
#endif

            _shaderResourceView->Release();
            _shaderResourceView = nullptr;
        }

        if (_resource)
        {
#if defined(_DEBUG)
            ULONG refCount = GetRefCount(_resource);
            ALIMER_ASSERT_MSG(refCount == 1, "D3D11Texture leakage");
#endif

            _resource->Release();
            _resource = nullptr;
        }
    }
}
