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

#include "graphics/SwapChain.h"

namespace alimer
{
    SwapChain::SwapChain(GraphicsDevice* device, const SwapChainDescriptor* descriptor)
        : _graphicsDevice(device)
        , _width(descriptor->width)
        , _height(descriptor->height)
        , _srgb(descriptor->srgb)
        , _colorFormat(descriptor->srgb ? PixelFormat::BGRA8UNormSrgb : PixelFormat::BGRA8UNorm)
        , _depthFormat(descriptor->depthFormat)
        , _vSyncEnabled(descriptor->vSyncEnabled)
    {
    }

    void SwapChain::Resize(uint32_t width, uint32_t height)
    {
        if (_width == width && _height == height)
        {
            return;
        }

        ResizeImpl(width, height);
        _width = width;
        _height = height;
    }
}
