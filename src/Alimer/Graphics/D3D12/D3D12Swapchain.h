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

#include "BackendD3D12.h"
#include "../../Application/Window.h"
#include "../Framebuffer.h"

namespace alimer
{
    class D3D12Texture;
    class D3D12Framebuffer;

	/// D3D12 Swapchain.
	class SwapChainD3D12 final
	{
	public:
		/// Constructor.
        SwapChainD3D12(GraphicsDeviceD3D12* device, const SwapChainDescriptor* descriptor);

		/// Destructor.
		~SwapChainD3D12();

        void Resize(uint32_t width, uint32_t height);
        void AfterReset();
        void Present();

        Framebuffer* GetFramebuffer() const {
            return _backBufferFramebuffers[_backBufferIndex];
        }

	private:
        static constexpr uint32_t   NumBackBuffers = 2;

        GraphicsDeviceD3D12*        _device;
        HWND                        _hwnd = nullptr;
        IUnknown*                   _window = nullptr;
        PixelFormat                 _backBufferFormat = PixelFormat::BGRA8UNorm;
        DXGI_FORMAT                 _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        UINT                        _swapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        UINT                        _syncInterval = 1;
        UINT                        _presentFlags = 0;

        IDXGISwapChain3*            _swapChain = nullptr;
        uint32_t                    _backBufferIndex = 0;
        SharedPtr<Texture>          _backBufferTextures[NumBackBuffers];
        SharedPtr<Framebuffer>      _backBufferFramebuffers[NumBackBuffers];
	};
}
