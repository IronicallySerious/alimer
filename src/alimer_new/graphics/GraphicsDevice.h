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

#include "graphics/SwapChain.h"

namespace alimer
{
    class ALIMER_API GraphicsDevice
    {
    protected:
        GraphicsDevice();

    public:
        /// Destructor.
        virtual ~GraphicsDevice() = default;

        /// Create GraphicsDevice instance.
        static GraphicsDevice* Create(const GraphicsDeviceDescriptor* descriptor);

        SwapChain* CreateSwapChain(SwapChainSurface* surface, const SwapChainDescriptor* descriptor);

        /// Get the device features.
        const GraphicsDeviceCapabilities& GetCaps() const { return _caps; }

        /// Get the backend.
        GraphicsBackend GetBackend() const { return _info.backend; }

        /// Get if gpu validation is enabled.
        bool IsValidationEnabled() const { return _validation; }

    protected:
        virtual SwapChain* CreateSwapChainImpl(SwapChainSurface* surface, const SwapChainDescriptor* descriptor) = 0;

    protected:
        GraphicsDeviceInfo          _info = {};
        GraphicsDeviceCapabilities  _caps = {};
        bool _validation = false;
    };

    ALIMER_API GraphicsBackend GetDefaultGraphicsPlatform();
    ALIMER_API bool IsGraphicsBackendSupported(GraphicsBackend backend);
}
