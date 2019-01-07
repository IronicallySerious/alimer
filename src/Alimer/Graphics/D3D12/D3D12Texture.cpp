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
#include "../../Debug/Log.h"

namespace Alimer
{
    D3D12Texture::D3D12Texture(D3D12Graphics* graphics, ID3D12Resource* resource)
        : TextureImpl()
    {
        if (resource)
        {
            _resource = resource;
            D3D12_RESOURCE_DESC desc = resource->GetDesc();
            switch (desc.Dimension)
            {
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                type = TextureType::Type1D;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                if (desc.DepthOrArraySize == 6)
                {
                    type = TextureType::TypeCube;
                }
                else 
                {
                    type = TextureType::Type2D;
                }
                break;

            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                type = TextureType::Type3D;
                break;

            default:
                break;
            }

            format = GetPixelFormatDxgiFormat(desc.Format);
            width = static_cast<uint32_t>(desc.Width);
            height = static_cast<uint32_t>(desc.Height);
            if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
            {
                depth = desc.DepthOrArraySize;
                arrayLayers = 1;
            }
            else
            {
                depth = 1;
                arrayLayers = desc.DepthOrArraySize;
            }
            mipLevels = desc.MipLevels;
            samples = static_cast<SampleCount>(desc.SampleDesc.Count);

            usage = TextureUsage::Unknown;
            if (!(desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
            {
                usage |= TextureUsage::ShaderRead;
            }

            if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
            {
                usage |= TextureUsage::ShaderWrite;
            }

            if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
                || desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
            {
                usage |= TextureUsage::RenderTarget;
            }
        }
    }

    D3D12Texture::~D3D12Texture()
    {
    }
}
