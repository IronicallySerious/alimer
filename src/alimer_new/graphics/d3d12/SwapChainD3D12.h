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

#include "BackendD3D12.h"
#include "graphics/SwapChain.h"

namespace alimer
{
    class ALIMER_API SwapChainD3D12 final : public SwapChain
    {
    public:
        SwapChainD3D12(GraphicsDeviceD3D12* device, const SwapChainSurface* surface, const SwapChainDescriptor* descriptor);
        ~SwapChainD3D12() override;

        bool ResizeImpl(uint32_t width, uint32_t height) override;
        bool GetNextTextureImpl() override;

        void PresentImpl() override;

    private:
        static constexpr uint32_t BufferCount = 2;

        // Surface data.
#if ALIMER_PLATFORM_UWP
#else
        HINSTANCE _hInstance;
        HWND _hWnd;
#endif

        DXGI_FORMAT _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        DXGI_SWAP_EFFECT _swapEffect;
        UINT _swapChainFlags;
        UINT _syncInterval;
        UINT _presentFlags;

        IDXGISwapChain3* _swapChain = nullptr;
    };
}
