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
#include "graphics/Graphics.h"
#include "math/vec2.h"

namespace alimer
{
    class Window;

    class GraphicsImpl final : public Graphics
    {
    public:
        static bool IsSupported();

        GraphicsImpl(const ApplicationConfiguration& config);
        ~GraphicsImpl();

        bool Initialize();
        void Shutdown() override;

        bool Resize(const math::uint2& newSize);

        bool BeginFrameImpl() override;
        void CommitFrame() override;

        const GraphicsDeviceInfo& GetInfo() const { return _info; }
        const GraphicsDeviceCapabilities& GetCaps() const { return _caps; }

        ID3D11Device1*          GetD3DDevice() const { return _d3dDevice.Get(); }
        ID3D11DeviceContext1*   GetD3DDeviceContext() const { return _d3dContext.Get(); }
        IDXGISwapChain1*        GetSwapChain() const { return _swapChain.Get(); }
        D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const { return _d3dFeatureLevel; }

        PixelFormat GetDefaultDepthStencilFormat() const;
        PixelFormat GetDefaultDepthFormat() const;
        bool IsInitialized() const;

    private:
        static void GetHardwareAdapter(Microsoft::WRL::ComPtr<IDXGIFactory1> factory, IDXGIAdapter1** ppAdapter);
        void CreateFactory();
        void CreateDeviceResources();
        bool CreateWindowSizeDependentResources();
        void InitializeCaps(IDXGIAdapter1* adapter);
        void UpdateColorSpace();
        void HandleDeviceLost();

    private:
        Microsoft::WRL::ComPtr<IDXGIFactory2>               _dxgiFactory;
        bool                                                _tearing = true;
        D3D_FEATURE_LEVEL                                   _d3dFeatureLevel  = D3D_FEATURE_LEVEL_9_1;
        Microsoft::WRL::ComPtr<ID3D11Device1>               _d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1>        _d3dContext;
        Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation>   _d3dAnnotation;

        DXGI_FORMAT                                         _backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        DXGI_FORMAT                                         _depthBufferFormat = DXGI_FORMAT_D32_FLOAT;
        UINT                                                _backBufferCount = 2;
        Microsoft::WRL::ComPtr<IDXGISwapChain1>             _swapChain;
        Microsoft::WRL::ComPtr<ID3D11Texture2D>             _renderTarget;
        Microsoft::WRL::ComPtr<ID3D11Texture2D>             _depthStencil;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      _d3dRenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      _d3dDepthStencilView;

        /// Current size of the backbuffer.
        math::uint2 _backbufferSize;

        // HDR Support
        DXGI_COLOR_SPACE_TYPE                           _colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

#if ALIMER_PLATFORM_UWP
        IUnknown*   _window = nullptr;
#else
        HWND        _window = nullptr;
#endif
    };
}
