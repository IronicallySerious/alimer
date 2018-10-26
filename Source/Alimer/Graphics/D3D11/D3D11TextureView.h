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

#include "Graphics/Texture.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
    class D3D11Texture;

    /// D3D11 TextureView implementation.
    class D3D11TextureView final : public TextureView
    {
    public:
        D3D11TextureView(const D3D11Texture* texture, const TextureViewDescriptor* descriptor);

        ~D3D11TextureView() override;

        ID3D11ShaderResourceView* GetSRV() const;
        ID3D11RenderTargetView* GetRTV() const;
        ID3D11DepthStencilView* GetDSV() const;

    private:
        const D3D11Texture* _d3dTexture;
        TextureUsage _textureUsage;
        mutable Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv;
        mutable Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _rtv;
        mutable Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _dsv;
    };
}
