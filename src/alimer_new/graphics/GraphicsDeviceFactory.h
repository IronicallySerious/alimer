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
    enum class PowerPreference
    {
        Default = 0,
        LowPower = 1,
        HighPerformance = 2,
    };

    struct AdapterDescriptor
    {
        PowerPreference powerPreference = PowerPreference::Default;
    };

    enum class GraphicsBackend
    {
        Default = 0,
        Null,
        Direct3D12
    };

    class GraphicsDevice;
    class ALIMER_API GraphicsDeviceFactory
    {
    protected:
        GraphicsDeviceFactory(GraphicsBackend backend);

    public:
        /// Destructor.
        virtual ~GraphicsDeviceFactory() = default;

        /// Create GraphicsDeviceFactory instance.
        static GraphicsDeviceFactory* Create(GraphicsBackend preferredBackend, bool validation);

        GraphicsDevice* CreateDevice(const AdapterDescriptor* adapterDescription);
        SwapChainSurface* CreateSurfaceFromWin32(void *hInstance, void *hWnd);

        bool Validation() const { return _validation; }

    protected:
        virtual GraphicsDevice* CreateDeviceImpl(const AdapterDescriptor* adapterDescription) = 0;
        virtual SwapChainSurface* CreateSurfaceFromWin32Impl(void *hInstance, void *hWnd) = 0;

        GraphicsBackend _backend = GraphicsBackend::Null;
        bool _validation = false;
    };
}
