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
#include "graphics/GraphicsDevice.h"

namespace alimer
{
    class GraphicsDeviceFactoryD3D12;

    /// D3D12 backend
    class ALIMER_API GraphicsDeviceD3D12 final : public GraphicsDevice
    {
    public:
        static bool IsSupported();

        GraphicsDeviceD3D12(PowerPreference powerPreference, bool validation);
        ~GraphicsDeviceD3D12() override;

        SwapChain* CreateSwapChainImpl(SwapChainSurface* surface, const SwapChainDescriptor* descriptor) override;

    private:
        void InitializeCaps();
        static void GetAdapter(_In_ IDXGIFactory2* factory, PowerPreference preference, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
        
    private:
        IDXGIFactory4* _factory = nullptr;
        bool _allowTearing = false;
        IDXGIAdapter1* _adapter = nullptr;
        ID3D12Device* _device = nullptr;
        D3D_FEATURE_LEVEL _featureLevel = D3D_FEATURE_LEVEL_9_1;
    };
}
