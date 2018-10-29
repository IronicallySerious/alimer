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

#include "../Framebuffer.h"
#include "D3D11Prerequisites.h"
#include <map>

namespace Alimer
{
    class D3D11GraphicsDevice;

    /// D3D11 Framebuffer implementation.
    class D3D11Framebuffer final : public Framebuffer
    {
    public:
        D3D11Framebuffer(D3D11GraphicsDevice* device, const FramebufferDescriptor* descriptor);

        uint32_t Bind(ID3D11DeviceContext* context) const;
        ID3D11RenderTargetView* GetColorRTV(uint32_t index) const
        {
            return _colorRtvs[index];
        }

        ID3D11DepthStencilView* GetDSV() const
        {
            return _depthStencilView;
        }

    private:
        uint32_t _viewsCount = 0;
        ID3D11RenderTargetView* _colorRtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
        ID3D11DepthStencilView* _depthStencilView = nullptr;
    };

}
