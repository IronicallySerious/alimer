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

#pragma once

#include "../Framebuffer.h"
#include "BackendD3D11.h"
#include <map>

namespace alimer
{
    /// D3D11 Framebuffer implementation.
    class FramebufferD3D11 final : public Framebuffer
    {
    public:
        FramebufferD3D11(GraphicsDeviceD3D11* device, const FramebufferDescriptor* descriptor);
        ~FramebufferD3D11() override;

        void Destroy();

        //void SetColorAttachment(uint32_t index, GPUTexture* colorTexture, uint32_t level, uint32_t slice) override;
        //void SetDepthStencilAttachment(GPUTexture* depthStencilTexture, uint32_t level, uint32_t slice) override;

        uint32_t Bind(ID3D11DeviceContext* context) const;

        ID3D11RenderTargetView* GetColorRTV(uint32_t index) const { return _colorRtvs[index]; }
        ID3D11DepthStencilView* GetDSV() const { return _depthStencilView; }
        //GPUTexture* GetDepthStencilTexture() const { return _depthStencilTexture; }

    private:
        //GPUTexture* _depthStencilTexture;

        uint32_t _colorAttachmentsCount = 0;
        ID3D11RenderTargetView* _colorRtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
        ID3D11DepthStencilView* _depthStencilView = nullptr;
    };

}
