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
#include "D3D11GraphicsDevice.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"

namespace Alimer
{
    D3D11Texture::D3D11Texture(D3D11Graphics* graphics, const TextureDescriptor* descriptor, const ImageLevel* initialData, ID3D11Texture2D* nativeTexture)
        : Texture(graphics, descriptor)
    {
        if (nativeTexture)
        {
            _dxgiFormat = d3d::Convert(descriptor->format, false);
            _texture2D = nativeTexture;
            return;
        }

        // Setup initial data.
        std::vector<D3D11_SUBRESOURCE_DATA> subResourceData;
        if (initialData)
        {
            subResourceData.resize(descriptor->arrayLayers * descriptor->mipLevels);
            for (uint32_t i = 0; i < descriptor->arrayLayers * descriptor->mipLevels; ++i)
            {
                uint32_t rowPitch;
                if (!initialData[i].rowPitch)
                {
                    const uint32_t mipWidth = ((descriptor->width >> i) > 0) ? descriptor->width >> i : 1;
                    const uint32_t mipHeight = ((descriptor->height >> i) > 0) ? descriptor->height >> i : 1;

                    uint32_t rows;
                    CalculateDataSize(
                        mipWidth,
                        mipHeight,
                        descriptor->format,
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

        const bool srgb = descriptor->colorSpace == TextureColorSpace::sRGB;
        _dxgiFormat = d3d::Convert(descriptor->format, srgb);

        switch (descriptor->type)
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
            d3d11Desc.Width = descriptor->width;
            d3d11Desc.Height = descriptor->height;
            d3d11Desc.MipLevels = descriptor->mipLevels;
            d3d11Desc.ArraySize = descriptor->arrayLayers;
            d3d11Desc.Format = _dxgiFormat;
            d3d11Desc.SampleDesc.Count = static_cast<uint32_t>(descriptor->samples);
            d3d11Desc.Usage = D3D11_USAGE_DEFAULT;

            if (descriptor->usage & TextureUsage::ShaderRead)
            {
                d3d11Desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }

            if (descriptor->usage & TextureUsage::ShaderWrite)
            {
                d3d11Desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
            }

            if (descriptor->usage & TextureUsage::RenderTarget)
            {
                if (!IsDepthStencilFormat(descriptor->format))
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
            if (descriptor->type == TextureType::TypeCube)
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
        if (descriptor->usage & TextureUsage::ShaderRead)
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

    D3D11Texture::~D3D11Texture()
    {
        Destroy();
    }

    void D3D11Texture::Destroy()
    {
        SafeRelease(_shaderResourceView, "ID3D11ShaderResourceView");
        SafeRelease(_resource, "ID3D11Texture");
    }
}
