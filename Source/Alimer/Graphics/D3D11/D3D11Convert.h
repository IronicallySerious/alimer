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

#pragma once

#include "D3D11Prerequisites.h"
#include "../Types.h"
#include "../PixelFormat.h"
#include "../Buffer.h"
#include "../Texture.h"
#include "../Sampler.h"

namespace Alimer
{
    namespace d3d11
    {
        static inline D3D11_USAGE Convert(ResourceUsage usage)
        {
            switch (usage)
            {
            case ResourceUsage::Default:    return D3D11_USAGE_DEFAULT;
            case ResourceUsage::Immutable:  return D3D11_USAGE_IMMUTABLE;
            case ResourceUsage::Dynamic:    return D3D11_USAGE_DYNAMIC;
            case ResourceUsage::Staging:    return D3D11_USAGE_STAGING;
            default:
                return D3D11_USAGE_DEFAULT;
            }
        }

        static inline D3D11_COMPARISON_FUNC Convert(CompareFunction function)
        {
            switch (function)
            {
            case CompareFunction::Never:
                return D3D11_COMPARISON_NEVER;

            case CompareFunction::Less:
                return D3D11_COMPARISON_LESS;

            case CompareFunction::Equal:
                return D3D11_COMPARISON_EQUAL;

            case CompareFunction::LessEqual:
                return D3D11_COMPARISON_LESS_EQUAL;

            case CompareFunction::Greater:
                return D3D11_COMPARISON_GREATER;

            case CompareFunction::NotEqual:
                return D3D11_COMPARISON_NOT_EQUAL;

            case CompareFunction::GreaterEqual:
                return D3D11_COMPARISON_GREATER_EQUAL;

            case CompareFunction::Always:
                return D3D11_COMPARISON_ALWAYS;

            default:
                ALIMER_UNREACHABLE();
            }
        }

        static inline D3D11_TEXTURE_ADDRESS_MODE Convert(SamplerAddressMode mode)
        {
            switch (mode)
            {
            case SamplerAddressMode::Repeat:
                return D3D11_TEXTURE_ADDRESS_WRAP;

            case SamplerAddressMode::MirrorRepeat:
                return D3D11_TEXTURE_ADDRESS_MIRROR;

            case SamplerAddressMode::ClampToEdge:    
                return D3D11_TEXTURE_ADDRESS_CLAMP;

            case SamplerAddressMode::ClampToBorder:   
                return D3D11_TEXTURE_ADDRESS_BORDER;

            case SamplerAddressMode::MirrorClampToEdge:
                return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;

            default:
                ALIMER_UNREACHABLE();
            }
        }

        static inline D3D11_FILTER_TYPE Convert(SamplerMinMagFilter filter)
        {
            switch (filter)
            {
            case SamplerMinMagFilter::Nearest:
                return D3D11_FILTER_TYPE_POINT;
            case SamplerMinMagFilter::Linear:
                return D3D11_FILTER_TYPE_LINEAR;
            default:
                ALIMER_UNREACHABLE();
                return (D3D11_FILTER_TYPE)-1;
            }
        }

        static inline D3D11_FILTER_TYPE Convert(SamplerMipFilter filter)
        {
            switch (filter)
            {
            case SamplerMipFilter::Nearest:
                return D3D11_FILTER_TYPE_POINT;
            case SamplerMipFilter::Linear:
                return D3D11_FILTER_TYPE_LINEAR;
            default:
                ALIMER_UNREACHABLE();
                return (D3D11_FILTER_TYPE)-1;
            }
        }

        static inline D3D11_FILTER Convert(SamplerMinMagFilter minFilter, SamplerMinMagFilter magFilter, SamplerMipFilter mipFilter, bool isComparison, bool isAnisotropic)
        {
            D3D11_FILTER filter;
            D3D11_FILTER_REDUCTION_TYPE reduction = isComparison ? D3D11_FILTER_REDUCTION_TYPE_COMPARISON : D3D11_FILTER_REDUCTION_TYPE_STANDARD;

            if (isAnisotropic)
            {
                filter = D3D11_ENCODE_ANISOTROPIC_FILTER(reduction);
            }
            else
            {
                D3D11_FILTER_TYPE dxMin = Convert(minFilter);
                D3D11_FILTER_TYPE dxMag = Convert(magFilter);
                D3D11_FILTER_TYPE dxMip = Convert(mipFilter);
                filter = D3D11_ENCODE_BASIC_FILTER(dxMin, dxMag, dxMip, reduction);
            }

            return filter;
        }
    }
}
