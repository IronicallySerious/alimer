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

#include "BackendD3D11.h"
#include "graphics/Types.h"

namespace alimer
{
    /// D3D12 backend
    class GraphicsImpl final 
    {
    public:
        static bool IsSupported();

        GraphicsImpl();
        ~GraphicsImpl();

        bool BeginFrame();
        void EndFrame();

        const GraphicsDeviceInfo& GetInfo() const { return _info; }
        const GraphicsDeviceCapabilities& GetCaps() const { return _caps; }

        ID3D11Device1*          GetD3DDevice() const { return _d3dDevice.Get(); }
        ID3D11DeviceContext1*   GetD3DDeviceContext() const { return _d3dContext.Get(); }
        IDXGISwapChain1*        GetSwapChain() const { return _swapChain.Get(); }
        D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const { return _d3dFeatureLevel; }

        PixelFormat GetDefaultDepthStencilFormat() const;
        PixelFormat GetDefaultDepthFormat() const;

    private:
        void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
        void InitializeCaps(IDXGIAdapter1* adapter);

    private:
        Microsoft::WRL::ComPtr<IDXGIFactory1>               _dxgiFactory;
        bool                                                _allowTearing = false;
        D3D_FEATURE_LEVEL                                   _d3dFeatureLevel  = D3D_FEATURE_LEVEL_9_1;
        Microsoft::WRL::ComPtr<ID3D11Device1>               _d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1>        _d3dContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain1>             _swapChain;
        Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation>   _d3dAnnotation;

        GraphicsDeviceInfo _info = {};
        GraphicsDeviceCapabilities  _caps = {};
    };
}
