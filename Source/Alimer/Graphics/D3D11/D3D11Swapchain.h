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
#include "../RenderWindow.h"
#include "../Framebuffer.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
    class D3D11Texture;
    class D3D11Framebuffer;
    class D3D11Graphics;

    /// D3D11 Swapchain implementation.
    class D3D11Swapchain final : public RenderWindow
    {
    public:
        /// Constructor.
        D3D11Swapchain(D3D11Graphics* graphics, const RenderWindowDescriptor* descriptor, uint32_t backBufferCount = 2);

        /// Destructor.
        ~D3D11Swapchain();

        void Destroy();

        void Resize(uint32_t width, uint32_t height, bool force = false);
        void SwapBuffers();

        Framebuffer* GetCurrentFramebuffer() const;
        uint32_t     GetBackBufferCount() const { return _backBufferCount; }

    private:
        D3D11Graphics* _graphics;
        uint32_t _width = 0;
        uint32_t _height = 0;
        uint32_t _backBufferCount = 2u;
        DXGI_FORMAT _backBufferFormat;
        PixelFormat _depthStencilFormat = PixelFormat::Unknown;
        HWND _hwnd = nullptr;
        IUnknown* _window = nullptr;

        UINT _swapChainFlags = 0;
        UINT _syncInterval = 1;
        UINT _presentFlags = 0;
        Microsoft::WRL::ComPtr<IDXGISwapChain>      _swapChain;
        Microsoft::WRL::ComPtr<IDXGISwapChain1>     _swapChain1;

        SharedPtr<Texture> _renderTarget;
        SharedPtr<Texture> _depthStencil;
        std::vector<SharedPtr<Framebuffer>> _framebuffers;
        uint32_t _currentBackBufferIndex = 0;
    };
}
