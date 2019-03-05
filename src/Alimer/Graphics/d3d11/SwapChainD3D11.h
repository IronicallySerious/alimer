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

#include "../Window.h"
#include "BackendD3D11.h"

namespace alimer
{
    /// D3D11 SwapChain implementation.
    class SwapChainD3D11 final : public Window
    {
    public:
        /// Constructor.
        SwapChainD3D11(GraphicsDeviceD3D11* device, const SwapChainDescriptor* descriptor);

        /// Destructor.
        ~SwapChainD3D11() override;

        void Destroy();

        void OnHandleCreated() override;
        void OnSizeChanged(const IntVector2& newSize) override;
        void ResizeImpl(uint32_t width, uint32_t height);
        void SwapBuffers() override;
        Texture* GetCurrentTexture() const { return _backbufferTexture; }
        Texture* GetDepthStencilTexture() const { return _depthStencilTexture; }
        Texture* GetMultisampleColorTexture() const { return _multisampleColorTexture; }

    private:
        static constexpr uint32_t   NumBackBuffers = 2;

        GraphicsDeviceD3D11*    _device;
        WindowHandle            _handle;

        PixelFormat     _backBufferFormat;
        DXGI_FORMAT     _dxgiBackBufferFormat;
        PixelFormat     _depthStencilFormat = PixelFormat::Undefined;
        UINT            _swapChainFlags;
        UINT            _syncInterval;
        UINT            _presentFlags;
        UINT            _sampleCount = 1;
        Microsoft::WRL::ComPtr<IDXGISwapChain>  _swapChain;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> _swapChain1;
        Texture*  _backbufferTexture = nullptr;
        Texture*  _depthStencilTexture = nullptr;
        Texture*  _multisampleColorTexture = nullptr;
    };
}
