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

#include "../SwapChain.h"
#include "BackendD3D11.h"

namespace alimer
{
    class TextureD3D11;
    class FramebufferD3D11;

    /// D3D11 SwapChain implementation.
    class SwapChainD3D11 final : public SwapChain
    {
    public:
        /// Constructor.
        SwapChainD3D11(DeviceD3D11* device, const SwapChainDescriptor* descriptor, uint32_t backBufferCount = 2);

        /// Destructor.
        ~SwapChainD3D11() override;

        void Destroy() override;

        void ResizeImpl(uint32_t width, uint32_t height) override;
        void PresentImpl() override;

    private:
        uint32_t                                _backBufferCount = 2u;
        HWND                                    _hwnd;
        IUnknown*                               _window;

        UINT                                    _swapChainFlags = 0;
        UINT                                    _syncInterval = 1;
        UINT                                    _presentFlags = 0;
        Microsoft::WRL::ComPtr<IDXGISwapChain>  _swapChain;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> _swapChain1;
    };
}
