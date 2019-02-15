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

#include "D3D12Texture.h"
#include "D3D12Graphics.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"

namespace alimer
{
    D3D12_RESOURCE_DIMENSION GetD3D12ResourceDimension(TextureType type)
    {
        switch (type)
        {
        case TextureType::Type1D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE1D;

        case TextureType::Type2D:
        case TextureType::TypeCube:
            return D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        case TextureType::Type3D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default:
            ALIMER_UNREACHABLE();
            return D3D12_RESOURCE_DIMENSION_UNKNOWN;
        }
    }


    D3D12_RESOURCE_FLAGS GetD3D12TextureUsage(TextureUsage usage, PixelFormat format)
    {
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

        bool uavRequired = any(usage & TextureUsage::ShaderWrite)/* || any(usage & TextureUsage::AccelerationStructure)*/;

        if (uavRequired)
        {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        if (any(usage & TextureUsage::RenderTarget))
        {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            if (IsDepthStencilFormat(format)) {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            }
        }

        if (!any(usage & TextureUsage::ShaderRead))
        {
            flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }

        return flags;
    }

    TextureD3D12::TextureD3D12(GraphicsDeviceD3D12* device, ID3D12Resource* resource)
        : Texture(device)
    {
        if (resource)
        {
            _externalHandle = true;
            _resource = resource;
            D3D12_RESOURCE_DESC desc = resource->GetDesc();
            switch (desc.Dimension)
            {
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                _type = TextureType::Type1D;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                if ((desc.DepthOrArraySize % 6) == 0)
                {
                    _type = TextureType::TypeCube;
                }
                else
                {
                    _type = TextureType::Type2D;
                }
                break;

            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                _type = TextureType::Type3D;
                break;

            default:
                break;
            }

            _format = GetPixelFormatDxgiFormat(desc.Format);
            _width = static_cast<uint32_t>(desc.Width);
            _height = static_cast<uint32_t>(desc.Height);
            if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
            {
                _depth = desc.DepthOrArraySize;
                _arraySize = 1;
            }
            else
            {
                _depth = 1;
                _arraySize = desc.DepthOrArraySize;
            }
            _mipLevels = desc.MipLevels;
            _samples = static_cast<SampleCount>(desc.SampleDesc.Count);

            _usage = TextureUsage::Unknown;
            if (!(desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
            {
                _usage |= TextureUsage::ShaderRead;
            }

            if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
            {
                _usage |= TextureUsage::ShaderWrite;
            }

            if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
                || desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
            {
                _usage |= TextureUsage::RenderTarget;
            }
        }
    }

    TextureD3D12::~TextureD3D12()
    {
        Destroy();
    }

    bool TextureD3D12::Create(const void* pInitData)
    {
        if (_externalHandle) {
            return true;
        }

        D3D12_RESOURCE_DESC resourceDescriptor;
        resourceDescriptor.Dimension = GetD3D12ResourceDimension(_type);
        resourceDescriptor.Alignment = 0;

        resourceDescriptor.Width = _width;
        resourceDescriptor.Height = _height;
        if (_type == TextureType::TypeCube)
        {
            resourceDescriptor.DepthOrArraySize = static_cast<UINT16>(_arraySize * 6);
        }
        else if (_type == TextureType::Type3D)
        {
            resourceDescriptor.DepthOrArraySize = static_cast<UINT16>(_depth);
        }
        else
        {
            resourceDescriptor.DepthOrArraySize = static_cast<UINT16>(_arraySize);
        }

        resourceDescriptor.MipLevels = static_cast<UINT16>(_mipLevels);
        resourceDescriptor.Format = GetDxgiFormat(_format);
        resourceDescriptor.SampleDesc.Count = static_cast<UINT>(_samples);
        resourceDescriptor.SampleDesc.Quality = 0;
        resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDescriptor.Flags = GetD3D12TextureUsage(_usage, _format);

        HRESULT hr = StaticCast<GraphicsDeviceD3D12>(_device)->GetD3DDevice()
            ->CreateCommittedResource(
                GetDefaultHeapProps(),
                D3D12_HEAP_FLAG_NONE,
                &resourceDescriptor,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&_resource));

        if (FAILED(hr)) {
            return false;
        }

        return true;
    }

    void TextureD3D12::Destroy()
    {
        if (_externalHandle) {
            return;
        }

#if !defined(NDEBUG)
        ULONG refCount = _resource.Reset();
        ALIMER_ASSERT_MSG(refCount == 0, "TextureD3D12 leakage");
#else
        _resource.Reset();
#endif
    }
}
