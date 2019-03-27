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

#include "graphics/Types.h"
#include "graphics/PixelFormat.h"

namespace alimer
{
    class GraphicsDevice;

    class ALIMER_API SwapChain
    {
    protected:
        SwapChain(GraphicsDevice* device, const SwapChainDescriptor* descriptor);

    public:
        /// Destructor.
        virtual ~SwapChain() = default;

        void Resize(uint32_t width, uint32_t height);
        bool GetNextTexture();
        void Present();
    private:
        virtual bool ResizeImpl(uint32_t width, uint32_t height) = 0;
        virtual bool GetNextTextureImpl() = 0;
        virtual void PresentImpl() = 0;

    protected:
        GraphicsDevice* _graphicsDevice;
        uint32_t _width;
        uint32_t _height;
        /// The color buffer format srgb.
        bool _srgb = true;
        PixelFormat _colorFormat = PixelFormat::BGRA8UNorm;
        /// The depth buffer format
        PixelFormat _depthFormat = PixelFormat::Depth32Float;
        /// vSync state
        bool _vSyncEnabled = false;
        uint32_t _currentBackBufferIndex = 0;
    };
}
