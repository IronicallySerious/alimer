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

#include "../Graphics.h"
#include "../../Application/Window.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
    class D3D11Texture;
    class D3D11RenderPass;
    class D3D11Graphics;

    enum class SwapchainFlagBits : uint32_t
    {
        None = 0,
        FlipPresent = 0x1,
        EnableHDR = 0x2,
    };
    using SwapchainFlags = Flags<SwapchainFlagBits, uint32_t>;
    ALIMER_FORCE_INLINE SwapchainFlags operator|(SwapchainFlagBits bit0, SwapchainFlagBits bit1)
    {
        return SwapchainFlags(bit0) | bit1;
    }

    ALIMER_FORCE_INLINE SwapchainFlags operator~(SwapchainFlagBits bits)
    {
        return ~(SwapchainFlags(bits));
    }

    /// D3D11 SwapChain implementation.
    class D3D11SwapChain final
    {
    public:
        /// Constructor.
        D3D11SwapChain(D3D11Graphics* graphics);

        /// Destructor.
        ~D3D11SwapChain();

        void SetWindow(HWND handle, uint32_t width, uint32_t height);
        void SetCoreWindow(IUnknown* window, uint32_t width, uint32_t height);

        void Resize(uint32_t width, uint32_t height, bool force = false);
        void Present();

        D3D11Texture* GetBackbufferTexture() const { return _backbufferTexture; }
        D3D11RenderPass* GetRenderPass() const { return _renderPass.Get(); }

    private:
        D3D11Graphics * _graphics;
        uint32_t _backBufferCount = 2u;
        uint32_t _width = 0;
        uint32_t _height = 0;
        HWND _hwnd = nullptr;
        IUnknown* _window = nullptr;
        DXGI_FORMAT _backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        bool _allowTearing = true;
        SwapchainFlags _swapchainFlags = SwapchainFlagBits::FlipPresent;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> _swapChain;
        D3D11Texture* _backbufferTexture = nullptr;
        SharedPtr<D3D11RenderPass> _renderPass;
    };
}
