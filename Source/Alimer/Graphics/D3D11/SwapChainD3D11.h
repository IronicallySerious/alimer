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

#include "../../Base/Ptr.h"
#include "../GPUDeviceImpl.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
    class DeviceD3D11;

    /// D3D11 SwapChain implementation.
    class SwapChainD3D11 final : public GPUSwapChain
    {
    public:
        /// Constructor.
        SwapChainD3D11(DeviceD3D11* device, void* window, uint32_t width, uint32_t height, PixelFormat depthStencilFormat, bool srgb, uint32_t backBufferCount = 2);

        /// Destructor.
        ~SwapChainD3D11();

        void Destroy();
    private:
        uint32_t GetBackBufferCount() const override { return 1; }
        uint32_t GetCurrentBackBuffer() const override { return 0; }
        GPUTexture* GetBackBufferTexture(uint32_t index) const
        {
            ALIMER_ASSERT(index == 0);
            return _renderTarget.Get();
        }

        void Configure(uint32_t width, uint32_t height) override;
        void Present() override;
        void Resize(uint32_t width, uint32_t height, bool force = false);

        DeviceD3D11*                            _device;
        uint32_t                                _width = 0;
        uint32_t                                _height = 0;
        uint32_t                                _backBufferCount = 2u;
        bool                                    _sRGB;
        DXGI_FORMAT                             _backBufferFormat = DXGI_FORMAT_UNKNOWN;
        PixelFormat                             _depthStencilFormat = PixelFormat::Unknown;
        HWND                                    _hwnd = nullptr;
        IUnknown*                               _window = nullptr;

        UINT                                    _swapChainFlags = 0;
        UINT                                    _syncInterval = 1;
        UINT                                    _presentFlags = 0;
        Microsoft::WRL::ComPtr<IDXGISwapChain>  _swapChain;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> _swapChain1;
        UniquePtr<GPUTexture>                   _renderTarget;
    };
}
