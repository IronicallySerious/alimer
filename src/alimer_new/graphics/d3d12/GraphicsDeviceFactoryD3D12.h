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

#include "BackendD3D12.h"
#include "graphics/GraphicsDeviceFactory.h"

namespace alimer
{
    /// D3D12 backend
    class ALIMER_API GraphicsDeviceFactoryD3D12 final : public GraphicsDeviceFactory
    {
    public:
        static bool IsSupported();
        /// Constructor.
        GraphicsDeviceFactoryD3D12(bool validation);
        /// Destructor.
        ~GraphicsDeviceFactoryD3D12() override;

        GraphicsDevice* CreateDeviceImpl(const AdapterDescriptor* adapterDescription) override;
        SwapChainSurface* CreateSurfaceFromWin32Impl(void *hInstance, void *hWnd) override;

    private:
        static void GetAdapter(_In_ IDXGIFactory2* factory, PowerPreference preference, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);

        Microsoft::WRL::ComPtr<IDXGIFactory4> _factory;
        bool _allowTearing = false;
    };
}
