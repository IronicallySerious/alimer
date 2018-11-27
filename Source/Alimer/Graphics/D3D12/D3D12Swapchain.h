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
#include "D3D12Prerequisites.h"

namespace Alimer
{
    class D3D12Texture;
	class D3D12Graphics;

	/// D3D12 Swapchain.
	class D3D12Swapchain final : public RefCounted
	{
	public:
		/// Constructor.
        D3D12Swapchain(D3D12Graphics* graphics, const SwapchainDescriptor* descriptor);

		/// Destructor.
		~D3D12Swapchain() override = default;

        void AfterReset();
        void Present();

	private:
        static constexpr uint32_t   NumBackBuffers = 2;

        D3D12Graphics*              _graphics;
        PixelFormat                 _backBufferFormat = PixelFormat::BGRA8UNorm;
        DXGI_FORMAT                 _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

        ComPtr<IDXGISwapChain3>     _swapChain;
        uint32_t                    _backBufferIndex = 0;
        UniquePtr<D3D12Texture>     _backBufferTextures[NumBackBuffers];
	};
}
