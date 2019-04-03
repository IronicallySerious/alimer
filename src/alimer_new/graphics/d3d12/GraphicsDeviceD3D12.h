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

        GraphicsDeviceD3D12(GpuPowerPreference powerPreference);
        ~GraphicsDeviceD3D12() override;

        PixelFormat GetDefaultDepthStencilFormat() const;
        PixelFormat GetDefaultDepthFormat() const;

        IDXGIFactory4* GetDXGIFactory() const { return _factory.Get(); }
        IDXGIAdapter1* GetDXGIAdapter() const { return _adapter.Get(); }
        bool AllowTearing() const { return _allowTearing; }

        ID3D12Device* GetD3DDevice() const { return _d3dDevice.Get(); }
        ID3D12CommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;

    private:
        // Backend methods
        SwapChain* CreateSwapChainImpl(const SwapChainSurface* surface, const SwapChainDescriptor* descriptor) override;

    private:
        void InitializeCaps();
        static void GetAdapter(_In_ IDXGIFactory2* factory, GpuPowerPreference preference, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
        
    private:
        ComPtr<IDXGIFactory4> _factory;
        bool _allowTearing = false;
        ComPtr<IDXGIAdapter1> _adapter;
        ComPtr<ID3D12Device> _d3dDevice;
        D3D_FEATURE_LEVEL _d3dMinFeatureLevel = D3D_FEATURE_LEVEL_11_0;
        D3D_FEATURE_LEVEL _d3dFeatureLevel = D3D_FEATURE_LEVEL_9_1;
    };
}
