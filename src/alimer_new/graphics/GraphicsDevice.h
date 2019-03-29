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

namespace alimer
{
    class Window;
    class SwapChain;

    class ALIMER_API GraphicsDevice
    {
    protected:
        /// Constructor.
        GraphicsDevice();

    public:
        /// Destructor.
        virtual ~GraphicsDevice() = default;

        static std::shared_ptr<GraphicsDevice> Create(GraphicsBackend preferredBackend, GpuPowerPreference powerPreference = GpuPowerPreference::Default);

        /// Create SwapChain with given surface and descriptor.
        SwapChain* CreateSwapChain(const SwapChainSurface* surface, const SwapChainDescriptor* descriptor);

        /// Create SwapChain from Window instance.
        SwapChain* CreateSwapChain(Window* window);

        /// Get the device info.
        const GraphicsDeviceInfo& GetInfo() const;

        /// Get the device capabilities.
        const GraphicsDeviceCapabilities& GetCaps() const;

    protected:
        virtual SwapChain* CreateSwapChainImpl(const SwapChainSurface* surface, const SwapChainDescriptor* descriptor) = 0;

    protected:
        GraphicsDeviceInfo _info = {};
        GraphicsDeviceCapabilities  _caps = {};
    };

    extern ALIMER_API std::shared_ptr<GraphicsDevice> graphicsDevice;
}
