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

#include "TextureD3D11.h"
#include "DeviceD3D11.h"
#include "D3D11Convert.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace alimer
{
    TextureD3D11::TextureD3D11(DeviceD3D11* device,
        const TextureDescriptor& descriptor, const void* initialData, 
        ID3D11Texture2D* externalTexture,  DXGI_FORMAT dxgiFormat)
        : _d3dDevice(device->GetD3DDevice())
        , _resource(nullptr)
        , _descriptor(descriptor)
    {
        if (externalTexture == nullptr)
        {
            // Setup initial data.
            PODVector<D3D11_SUBRESOURCE_DATA> subResourceData;
            if (initialData)
            {
                subResourceData.Resize(_descriptor.arraySize * _descriptor.mipLevels);
                for (uint32_t mipLevel = 0; mipLevel < _descriptor.arraySize * _descriptor.mipLevels; ++mipLevel)
                {
                    uint32_t rowPitch;
                    const uint32_t mipWidth = Max(1u, _descriptor.width >> mipLevel);
                    const uint32_t mipHeight = Max(1u, _descriptor.height >> mipLevel);

                    uint32_t rows;
                    CalculateDataSize(
                        mipWidth,
                        mipHeight,
                        _descriptor.format,
                        &rows,
                        &rowPitch);

                    subResourceData[mipLevel].pSysMem = initialData;
                    subResourceData[mipLevel].SysMemPitch = rowPitch;
                    subResourceData[mipLevel].SysMemSlicePitch = 0;
                }
            }


            // If depth stencil format and shader read or write, switch to typeless.
            if (IsDepthStencilFormat(_descriptor.format)
                && any(_descriptor.usage & (TextureUsage::Sampled | TextureUsage::Storage)))
            {
                _dxgiFormat = GetDxgiTypelessDepthFormat(_descriptor.format);
            }
            else
            {
                _dxgiFormat = GetDxgiFormat(_descriptor.format);
            }

            HRESULT hr = S_OK;
            D3D11_USAGE d3dUsage = D3D11_USAGE_DEFAULT;
            UINT d3dBindFlags = 0;
            const bool dynamic = false;
            UINT d3dCPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
            UINT d3dMiscFlags = 0;
            if (any(_descriptor.usage & TextureUsage::Sampled))
            {
                d3dBindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }

            if (any(_descriptor.usage & TextureUsage::Storage))
            {
                d3dBindFlags |= D3D11_BIND_UNORDERED_ACCESS;
            }

            if (any(_descriptor.usage & TextureUsage::OutputAttachment))
            {
                if (!IsDepthStencilFormat(_descriptor.format))
                {
                    d3dBindFlags |= D3D11_BIND_RENDER_TARGET;
                    if (_descriptor.mipLevels == 0 || _descriptor.mipLevels > 1)
                    {
                        d3dMiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
                    }
                }
                else
                {
                    d3dBindFlags |= D3D11_BIND_DEPTH_STENCIL;
                }
            }

            if (_descriptor.type == TextureType::TypeCube)
            {
                d3dMiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
            }

            switch (_descriptor.type)
            {
            case TextureType::Type1D:
            {
                D3D11_TEXTURE1D_DESC d3d11Desc;
                {
                    d3d11Desc.Width = _descriptor.width;
                    d3d11Desc.MipLevels = _descriptor.mipLevels;
                    d3d11Desc.ArraySize = _descriptor.arraySize;
                    d3d11Desc.Format = _dxgiFormat;
                    d3d11Desc.Usage = d3dUsage;
                    d3d11Desc.BindFlags = d3dBindFlags;
                    d3d11Desc.CPUAccessFlags = d3dCPUAccessFlags;
                    d3d11Desc.MiscFlags = d3dMiscFlags;
                };

                hr = _d3dDevice->CreateTexture1D(&d3d11Desc, subResourceData.Data(), &_texture1D);
            }
            break;

            case TextureType::Type2D:
            case TextureType::TypeCube:
            {
                const uint32_t arrayMultiplier = (_descriptor.type == TextureType::TypeCube) ? 6 : 1;
                D3D11_TEXTURE2D_DESC d3d11Desc = {};
                d3d11Desc.Width = _descriptor.width;
                d3d11Desc.Height = _descriptor.height;
                d3d11Desc.MipLevels = _descriptor.mipLevels;
                d3d11Desc.ArraySize = _descriptor.arraySize * arrayMultiplier;
                d3d11Desc.Format = _dxgiFormat;
                d3d11Desc.SampleDesc.Count = Max(1u, static_cast<uint32_t>(_descriptor.samples));
                d3d11Desc.Usage = d3dUsage;
                d3d11Desc.BindFlags = d3dBindFlags;
                d3d11Desc.CPUAccessFlags = d3dCPUAccessFlags;
                d3d11Desc.MiscFlags = d3dMiscFlags;
                hr = _d3dDevice->CreateTexture2D(&d3d11Desc, subResourceData.Data(), &_texture2D);

            }
            break;

            default:
                break;
            }
        }
        else
        {
            _texture2D = externalTexture;
            _dxgiFormat = dxgiFormat;
        }
    }

    TextureD3D11::~TextureD3D11()
    {
        Destroy();
    }

    void TextureD3D11::Destroy()
    {
        InvalidateViews();
        SafeRelease(_resource, "ID3D11Texture");
    }

    void TextureD3D11::InvalidateViews()
    {
        _srvs.clear();
        _uavs.clear();
    }

    template<typename ViewType>
    using CreateViewFunc = std::function<typename ComPtr<ViewType>(const TextureD3D11* texture, uint32_t mipLevel, uint32_t mipLevelCount, uint32_t firstArraySlice, uint32_t arraySize)>;

    template<typename ViewType, typename ViewMapType>
    typename ViewType* FindViewCommon(
        const TextureD3D11* texture,
        uint32_t mipLevel,
        uint32_t mipLevelCount,
        uint32_t firstArraySlice,
        uint32_t arraySize,
        ViewMapType& viewMap,
        CreateViewFunc<ViewType> createFunc)
    {
        uint32_t textureArraySize = texture->GetArrayLayers();
        uint32_t textureMipLevels = texture->GetMipLevels();

        if (firstArraySlice >= textureArraySize)
        {
            firstArraySlice = textureArraySize - 1;
        }

        if (mipLevel >= textureMipLevels)
        {
            mipLevel = textureMipLevels - 1;
        }

        if (mipLevelCount == RemainingMipLevels)
        {
            mipLevelCount = textureMipLevels - mipLevel;
        }
        else if (mipLevelCount + mipLevel > textureMipLevels)
        {
            mipLevelCount = textureMipLevels - mipLevel;
        }

        if (arraySize == RemainingArrayLayers)
        {
            arraySize = textureArraySize - firstArraySlice;
        }
        else if (arraySize + firstArraySlice > textureArraySize)
        {
            arraySize = textureArraySize - firstArraySlice;
        }

        auto viewInfo = D3DResourceViewInfo(mipLevel, mipLevelCount, firstArraySlice, arraySize);
        if (viewMap.find(viewInfo) == viewMap.end())
        {
            viewMap[viewInfo] = createFunc(texture, mipLevel, mipLevelCount, firstArraySlice, arraySize);
        }

        return viewMap[viewInfo].Get();
    }

    ID3D11ShaderResourceView* TextureD3D11::GetSRV(uint32_t mostDetailedMip, uint32_t mipCount, uint32_t firstArraySlice, uint32_t arraySize) const
    {
        return nullptr;
        /*auto createFunc = [](const TextureD3D11* texture, uint32_t mostDetailMip, uint32_t mipLevels, uint32_t firstArraySlice, uint32_t arraySize)
        {
            uint32_t arrayMultiplier = (texture->GetTextureType() == TextureType::TypeCube) ? 6 : 1;
            const uint32_t arrayLayers = texture->GetArrayLayers();
            const bool isTextureMs = static_cast<uint32_t>(texture->GetSamples()) > 1;

            D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
            desc.Format = texture->GetDXGIFormat();
            switch (texture->GetTextureType())
            {
            case TextureType::Type1D:
                if (arrayLayers > 1)
                {
                    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.MostDetailedMip = mostDetailMip;
                    desc.Texture1DArray.MipLevels = mipLevels;
                    desc.Texture1DArray.FirstArraySlice = firstArraySlice;
                    desc.Texture1DArray.ArraySize = arraySize;
                }
                else
                {
                    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MostDetailedMip = mostDetailMip;
                    desc.Texture1D.MipLevels = mipLevels;
                }
                break;
            case TextureType::Type2D:
                if (arrayLayers > 1)
                {
                    if (isTextureMs)
                    {
                        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
                        desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
                        desc.Texture2DMSArray.ArraySize = arraySize;
                    }
                    else
                    {
                        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                        desc.Texture2DArray.MostDetailedMip = mostDetailMip;
                        desc.Texture2DArray.MipLevels = mipLevels;
                        desc.Texture2DArray.FirstArraySlice = firstArraySlice * arrayMultiplier;
                        desc.Texture2DArray.ArraySize = arraySize * arrayMultiplier;
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
                        desc.Texture2D.MostDetailedMip = mostDetailMip;
                        desc.Texture2D.MipLevels = mipLevels;
                    }
                }

                break;

            case TextureType::Type3D:
                assert(arraySize == 1);
                desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
                desc.Texture3D.MostDetailedMip = mostDetailMip;
                desc.Texture3D.MipLevels = mipLevels;
                break;

            case TextureType::TypeCube:
                if (arraySize > 1)
                {
                    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
                    desc.TextureCubeArray.MostDetailedMip = mostDetailMip;
                    desc.TextureCubeArray.MipLevels = mipLevels;
                    desc.TextureCubeArray.First2DArrayFace = firstArraySlice;
                    desc.TextureCubeArray.NumCubes = arraySize;
                }
                else
                {
                    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                    desc.TextureCube.MostDetailedMip = mostDetailMip;
                    desc.TextureCube.MipLevels = mipLevels;
                }
                break;

            default:
                desc.ViewDimension = D3D11_SRV_DIMENSION_UNKNOWN;
                ALIMER_LOGCRITICAL("Invalid texture type");
                break;
            }

            ComPtr<ID3D11ShaderResourceView> view;
            HRESULT hr = texture->GetD3DDevice()->CreateShaderResourceView(
                texture->GetResource(),
                &desc,
                view.ReleaseAndGetAddressOf());
            if (FAILED(hr))
            {
                ALIMER_LOGCRITICAL("[D3D] - CreateShaderResourceView failed");
            }

            return view;
        };

        return FindViewCommon<ID3D11ShaderResourceView>(this, mostDetailedMip, mipCount, firstArraySlice, arraySize, _srvs, createFunc);*/
    }

    ID3D11UnorderedAccessView* TextureD3D11::GetUAV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize) const
    {
        return nullptr;
        /*auto createFunc = [](const TextureD3D11* texture, uint32_t mipLevel, uint32_t mipLevelCount, uint32_t firstArraySlice, uint32_t arraySize)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC  desc = {};
            desc.Format = texture->GetDXGIFormat();
            switch (texture->GetTextureType())
            {
            case TextureType::Type1D:
                if (texture->GetArrayLayers() > 1)
                {
                    desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.MipSlice = mipLevel;
                    desc.Texture1DArray.FirstArraySlice = firstArraySlice;
                    desc.Texture1DArray.ArraySize = arraySize;
                }
                else
                {
                    desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MipSlice = mipLevel;
                }
                break;
            case TextureType::Type2D:
                if (texture->GetArrayLayers() > 1)
                {
                    desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                    desc.Texture2DArray.MipSlice = mipLevel;
                    desc.Texture2DArray.FirstArraySlice = firstArraySlice;
                    desc.Texture2DArray.ArraySize = arraySize;
                }
                else
                {
                    desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = mipLevel;
                }

                break;

            case TextureType::Type3D:
                assert(texture->GetArrayLayers() == 1);
                desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
                desc.Texture3D.MipSlice = mipLevel;
                desc.Texture3D.FirstWSlice = firstArraySlice;
                desc.Texture3D.WSize = texture->GetDepth();
                break;

            case TextureType::TypeCube:
                assert(texture->GetArrayLayers() == 1);
                desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                desc.Texture2DArray.MipSlice = mipLevel;
                desc.Texture2DArray.FirstArraySlice = firstArraySlice;
                desc.Texture2DArray.ArraySize = arraySize;
                break;

            default:
                desc.ViewDimension = D3D11_UAV_DIMENSION_UNKNOWN;
                ALIMER_LOGCRITICAL("Texture type not supported for UAV creation");
                break;
            }

            ComPtr<ID3D11UnorderedAccessView> view;
            HRESULT hr = texture->GetD3DDevice()->CreateUnorderedAccessView(
                texture->GetResource(),
                &desc,
                view.ReleaseAndGetAddressOf());
            if (FAILED(hr))
            {
                ALIMER_LOGCRITICAL("[D3D] - CreateUnorderedAccessView failed");
            }

            return view;
        };

        return FindViewCommon<ID3D11UnorderedAccessView>(this, mipLevel, 1, firstArraySlice, arraySize, _uavs, createFunc);*/
    }
}
