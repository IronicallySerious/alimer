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

#include "SamplerD3D12.h"
#include "GraphicsDeviceD3D12.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"

namespace alimer
{
    D3D12_FILTER_TYPE GetD3D12Filter(SamplerMinMagFilter filter)
    {
        switch (filter)
        {
        case SamplerMinMagFilter::Nearest:
            return D3D12_FILTER_TYPE_POINT;
        case SamplerMinMagFilter::Linear:
            return D3D12_FILTER_TYPE_LINEAR;
        default:
            ALIMER_UNREACHABLE();
            return (D3D12_FILTER_TYPE)-1;
        }
    }

    D3D12_FILTER_TYPE GetD3D12Filter(SamplerMipFilter filter)
    {
        switch (filter)
        {
        case SamplerMipFilter::Nearest:
            return D3D12_FILTER_TYPE_POINT;
        case SamplerMipFilter::Linear:
            return D3D12_FILTER_TYPE_LINEAR;
        default:
            ALIMER_UNREACHABLE();
            return (D3D12_FILTER_TYPE)-1;
        }
    }

    D3D12_FILTER GetD3D12Filter(SamplerMinMagFilter minFilter, SamplerMinMagFilter magFilter, SamplerMipFilter mipFilter, bool isComparison, bool isAnisotropic)
    {
        D3D12_FILTER filter;
        D3D12_FILTER_REDUCTION_TYPE reduction = isComparison ? D3D12_FILTER_REDUCTION_TYPE_COMPARISON : D3D12_FILTER_REDUCTION_TYPE_STANDARD;

        if (isAnisotropic)
        {
            filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reduction);
        }
        else
        {
            D3D12_FILTER_TYPE dxMin = GetD3D12Filter(minFilter);
            D3D12_FILTER_TYPE dxMag = GetD3D12Filter(magFilter);
            D3D12_FILTER_TYPE dxMip = GetD3D12Filter(mipFilter);
            filter = D3D12_ENCODE_BASIC_FILTER(dxMin, dxMag, dxMip, reduction);
        }

        return filter;
    }

    D3D12_TEXTURE_ADDRESS_MODE GetD3D12AddressMode(SamplerAddressMode mode)
    {
        switch (mode)
        {
        case SamplerAddressMode::ClampToEdge:
            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

        case SamplerAddressMode::MirrorClampToEdge:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;

        case SamplerAddressMode::Repeat:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;

        case SamplerAddressMode::MirrorRepeat:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
       
        case SamplerAddressMode::ClampToBorder:
            return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        default:
            ALIMER_UNREACHABLE();
            return (D3D12_TEXTURE_ADDRESS_MODE)-1;
        }
    }

    SamplerD3D12::SamplerD3D12(GraphicsDeviceD3D12* device, const SamplerDescriptor* descriptor)
        : Sampler(device, descriptor)
    {
        Create();
    }

    SamplerD3D12::~SamplerD3D12()
    {
        Destroy();
    }

    bool SamplerD3D12::Create()
    {
        const bool isComparison = _descriptor.compareOp != CompareOp::Never;
        const bool isAnisotropic = _descriptor.maxAnisotropy > 1;
        _samplerDesc.Filter = GetD3D12Filter(
            _descriptor.minFilter,
            _descriptor.magFilter,
            _descriptor.mipmapFilter,
            isComparison,
            isAnisotropic);

        _samplerDesc.AddressU = GetD3D12AddressMode(_descriptor.addressModeU);
        _samplerDesc.AddressV = GetD3D12AddressMode(_descriptor.addressModeV);
        _samplerDesc.AddressW = GetD3D12AddressMode(_descriptor.addressModeV);
        _samplerDesc.MipLODBias = 0.0f;
        _samplerDesc.MaxAnisotropy = Max<UINT>(_descriptor.maxAnisotropy, D3D12_MAX_MAXANISOTROPY);
        _samplerDesc.ComparisonFunc = GetD3D12ComparisonFunc(_descriptor.compareOp);

        switch (_descriptor.borderColor)
        {
        case SamplerBorderColor::TransparentBlack:
            _samplerDesc.BorderColor[0] = _samplerDesc.BorderColor[1] = _samplerDesc.BorderColor[2] = _samplerDesc.BorderColor[3] = 0;
            break;
        case SamplerBorderColor::OpaqueBlack:
            _samplerDesc.BorderColor[0] = _samplerDesc.BorderColor[1] = _samplerDesc.BorderColor[2] = 0;
            _samplerDesc.BorderColor[3] = 1;
            break;
        case SamplerBorderColor::OpaqueWhite:
            _samplerDesc.BorderColor[0] = _samplerDesc.BorderColor[1] = _samplerDesc.BorderColor[2] = _samplerDesc.BorderColor[3] = 1;
            break;
        default:
            ALIMER_UNREACHABLE();
            break;
        }
        _samplerDesc.MinLOD = _descriptor.lodMinClamp;
        _samplerDesc.MaxLOD = _descriptor.lodMaxClamp;

        return true;
    }

    void SamplerD3D12::Destroy()
    {
    }
}
