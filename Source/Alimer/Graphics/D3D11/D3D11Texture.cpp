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
#include "D3D11TextureView.h"
#include "D3D11GraphicsDevice.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11Texture::D3D11Texture(D3D11GraphicsDevice* device, const TextureDescriptor* descriptor, const ImageLevel* initialData, ID3D11Texture2D* nativeTexture)
        : Texture(device, descriptor)
        , _d3dDevice(device->GetD3DDevice())
    {
        if (nativeTexture)
        {
            _dxgiFormat = GetDxgiFormat(descriptor->format);
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

        _dxgiFormat = GetDxgiFormat(descriptor->format);

        // If depth stencil format and shader read or write, switch to typeless.
        if (IsDepthStencilFormat(descriptor->format)
            && any(descriptor->usage & (TextureUsage::ShaderRead | TextureUsage::ShaderWrite)))
        {
            _dxgiFormat = GetDxgiTypelessDepthFormat(descriptor->format);
        }

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

            if (any(descriptor->usage & TextureUsage::ShaderRead))
            {
                d3d11Desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }

            if (any(descriptor->usage & TextureUsage::ShaderWrite))
            {
                d3d11Desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
            }

            if (any(descriptor->usage & TextureUsage::RenderTarget))
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

            _d3dDevice->CreateTexture2D(
                &d3d11Desc,
                subResourceData.data(),
                &_texture2D);

        }
        break;

        default:
            break;
        }
    }

    D3D11Texture::~D3D11Texture()
    {
        Destroy();
    }

    void D3D11Texture::Destroy()
    {
        InvalidateViews();
        SafeRelease(_resource, "ID3D11Texture");
    }

    SharedPtr<TextureView> D3D11Texture::CreateTextureView(const TextureViewDescriptor* descriptor) const
    {
        return MakeShared<D3D11TextureView>(this, descriptor);
    }
}
