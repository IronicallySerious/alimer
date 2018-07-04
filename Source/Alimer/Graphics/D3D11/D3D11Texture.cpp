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
    D3D11Texture::D3D11Texture(D3D11Graphics* graphics)
        : Texture(graphics)
        , _d3dDevice(graphics->GetD3DDevice())
        , _resource(nullptr)
        , _dxgiFormat(DXGI_FORMAT_UNKNOWN)
    {

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
