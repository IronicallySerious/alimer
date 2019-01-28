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

#include "../Base/Ptr.h"
#include "../Graphics/Backend.h"
#include "../Graphics/Framebuffer.h"

namespace alimer
{
    class GPUSwapChain;

	/// Defines a SwapChain.
	class ALIMER_API SwapChain final : public GPUResource
	{
        friend class GraphicsDevice;

    public:
        /// Constructor.
        SwapChain(GraphicsDevice* device);

        /// Destructor
        ~SwapChain() override;

        void Destroy() override;

        void Resize(uint32_t width, uint32_t height);
        void Present();

        uint32_t GetBackBufferCount() const { return _backbufferTextures.Size(); }
        Texture* GetBackBufferTexture(uint32_t index) const { return _backbufferTextures[index].Get(); }
        uint32_t GetCurrentBackBuffer() const { return _currentBackBuffer; }
        Framebuffer* GetCurrentFramebuffer() const { return _framebuffers[_currentBackBuffer].Get(); }

        /// Get the swap chain width.
        uint32_t GetWidth() const { return _width; }

        /// Get the swap chain height.
        uint32_t GetHeight() const { return _height; }

    private:
        GPUSwapChain* _impl = nullptr;

    protected:
        void InitializeFramebuffer();

        uint32_t                        _width = 0;
        uint32_t                        _height = 0;
        PixelFormat                     _colorFormat = PixelFormat::Unknown;
        PixelFormat                     _depthStencilFormat = PixelFormat::Unknown;
        Vector<SharedPtr<Texture>>      _backbufferTextures;
        uint32_t                        _currentBackBuffer = 0;
        SharedPtr<Texture>              _depthStencilTexture;
        Vector<SharedPtr<Framebuffer>>  _framebuffers;
    };
}
