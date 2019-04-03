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

#include "SwapChainD3D12.h"
#include "GraphicsDeviceD3D12.h"

namespace alimer
{
    SwapChainD3D12::SwapChainD3D12(GraphicsDeviceD3D12* device, const SwapChainSurface* surface, const SwapChainDescriptor* descriptor)
        : SwapChain(device, descriptor)
    {
#if ALIMER_PLATFORM_UWP
#else
        _hInstance = reinterpret_cast<HINSTANCE>(surface->hInstance);
        _hWnd = reinterpret_cast<HWND>(surface->window);
        if (!IsWindow(_hWnd))
        {
            ALIMER_ASSERT_MSG(false, "Invalid hWnd handle");
        }

        if (_width == 0
            || _height == 0)
        {
            RECT rect;
            GetClientRect(_hWnd, &rect);
            _width = static_cast<uint32_t>(rect.right - rect.left);
            _height = static_cast<uint32_t>(rect.bottom - rect.top);
        }

#endif

        if (descriptor->srgb)
        {
            _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        }

        _swapEffect = DXGI_SWAP_EFFECT_DISCARD;
        _swapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        _syncInterval = 0;
        _presentFlags = DXGI_PRESENT_DO_NOT_WAIT;

        if (device->AllowTearing())
        {
            _syncInterval = 0;
            _presentFlags = DXGI_PRESENT_ALLOW_TEARING;
            _swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
            _swapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            // Cannot use srgb format with flip swap effect.
            _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        }
        else
        {
            if (descriptor->vSyncEnabled)
            {
                _syncInterval = 1;
                _presentFlags = 0;
            }
        }

        ResizeImpl(_width, _height);
    }

    SwapChainD3D12::~SwapChainD3D12()
    {
    }

    bool SwapChainD3D12::ResizeImpl(uint32_t width, uint32_t height)
    {
        HRESULT hr = S_OK;

        if (_swapChain)
        {
            //SwapChain::Destroy();

            hr = _swapChain->ResizeBuffers(BufferCount, width, height, DXGI_FORMAT_UNKNOWN, _swapChainFlags);

            if (hr == DXGI_ERROR_DEVICE_REMOVED
                || hr == DXGI_ERROR_DEVICE_RESET)
            {
            }
        }
        else
        {
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.Width = width;
            swapChainDesc.Height = height;
            swapChainDesc.Format = _dxgiBackBufferFormat;
            swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = BufferCount;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = _swapEffect;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags = _swapChainFlags;

            auto deviceD3D12 = static_cast<GraphicsDeviceD3D12*>(_graphicsDevice);
            IDXGIFactory4* dxgiFactory = deviceD3D12->GetDXGIFactory();

            ComPtr<IDXGISwapChain1> swapChain;

#if ALIMER_PLATFORM_UWP
            ThrowIfFailed(dxgiFactory->CreateSwapChainForCoreWindow(
                deviceD3D12->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
                _window,
                &swapChainDesc,
                nullptr,
                &swapChain
            ));
#else
            /* TODO: DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT*/

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
            fsSwapChainDesc.Windowed = TRUE;

            // Create a swap chain for the window.
            ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
                deviceD3D12->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
                _hWnd,
                &swapChainDesc,
                &fsSwapChainDesc,
                nullptr,
                &swapChain
            ));



            // This sample does not support fullscreen transitions.
            ThrowIfFailed(dxgiFactory->MakeWindowAssociation(_hWnd, DXGI_MWA_NO_ALT_ENTER));
#endif

            ThrowIfFailed(swapChain.As(&_swapChain));
        }

        return true;
    }

    bool SwapChainD3D12::GetNextTextureImpl()
    {
        _currentBackBufferIndex = _swapChain->GetCurrentBackBufferIndex();
        return true;
    }

    void SwapChainD3D12::PresentImpl()
    {
        HRESULT hr = _swapChain->Present(_syncInterval, _presentFlags);

        // If the device was removed either by a disconnection or a driver upgrade, we must recreate all device resources.
        if (hr == DXGI_ERROR_DEVICE_REMOVED
            || hr == DXGI_ERROR_DEVICE_RESET)
        {
            //_graphicsDevice->HandleDeviceLost();
        }
        else
        {
            ThrowIfFailed(hr);
        }

        //_currentBackBufferIndex = _swapChain->GetCurrentBackBufferIndex();
    }
}
