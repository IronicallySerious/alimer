//
// Copyright (c) 2018 Amer Koleci and contributors.
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

#include "D3D11SwapChain.h"
#include "D3D11Graphics.h"
#include "D3D11Texture.h"
#include "D3D11RenderPass.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11SwapChain::D3D11SwapChain(D3D11Graphics* graphics)
        : _graphics(graphics)
    {
        // Determines whether tearing support is available for fullscreen borderless windows.
        ComPtr<IDXGIFactory4> factory4;
        ComPtr<IDXGIFactory5> factory5;
        bool validFactory4 = SUCCEEDED(graphics->GetDXGIFactory()->QueryInterface(IID_PPV_ARGS(&factory4)));
        bool validFactory5 = SUCCEEDED(graphics->GetDXGIFactory()->QueryInterface(IID_PPV_ARGS(&factory5)));

        // Check tearing support.
        BOOL allowTearing = FALSE;
        HRESULT hr = S_FALSE;
        if (validFactory5)
        {
            hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        }

        if (FAILED(hr) || !allowTearing)
        {
            _allowTearing = false;
            ALIMER_LOGWARN("D3D11: Variable refresh rate displays not supported");
        }

        // Disable HDR if we are on an OS that can't support FLIP swap effects
        if (_swapchainFlags & SwapchainFlagBits::EnableHDR)
        {
            ComPtr<IDXGIFactory5> factory5;
            if (!validFactory5)
            {
                _swapchainFlags &= ~SwapchainFlagBits::EnableHDR;
                ALIMER_LOGWARN("D3D11: HDR swap chains not supported");
            }
        }

        // Disable FLIP if not on a supporting OS
        if (_swapchainFlags & SwapchainFlagBits::FlipPresent)
        {
            if (!validFactory4)
            {
                _swapchainFlags &= ~SwapchainFlagBits::FlipPresent;
                ALIMER_LOGWARN("D3D11: Flip swap effects not supported");
            }
        }
    }

    D3D11SwapChain::~D3D11SwapChain()
    {
        SafeDelete(_backbufferTexture);
        _renderPass.Reset();
        _swapChain.Reset();
    }

    void D3D11SwapChain::SetWindow(HWND handle, uint32_t width, uint32_t height)
    {
        _hwnd = handle;
        Resize(width, height, true);
    }

    void D3D11SwapChain::SetCoreWindow(IUnknown* window, uint32_t width, uint32_t height)
    {
        _window = window;
        Resize(width, height, true);
    }

    void D3D11SwapChain::Resize(uint32_t width, uint32_t height, bool force)
    {
        if (_width == width &&
            _height == height &&
            !force) {
            return;
        }

        // Clear the previous window size specific context.
        ID3D11RenderTargetView* nullViews[] = { nullptr };
        _graphics->GetImmediateContext()->OMSetRenderTargets(1, nullViews, nullptr);

        UINT dxgiSwapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        if (_allowTearing)
        {
            dxgiSwapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        }

        if (_swapChain)
        {
            HRESULT hr = _swapChain->ResizeBuffers(
                _backBufferCount,
                width,
                height,
                _backBufferFormat,
                dxgiSwapChainFlags
            );

            if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
            {
            }
        }
        else
        {

            // Describe and create the swap chain.
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.Width = width;
            swapChainDesc.Height = height;
            swapChainDesc.Format = _backBufferFormat;
            swapChainDesc.Stereo = FALSE;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = _backBufferCount;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = _allowTearing || (_swapchainFlags & (SwapchainFlagBits::FlipPresent | SwapchainFlagBits::EnableHDR)) ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags = dxgiSwapChainFlags;

            if (!_window)
            {
                DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
                fsSwapChainDesc.Windowed = TRUE;

                ThrowIfFailed(
                    _graphics->GetDXGIFactory()->CreateSwapChainForHwnd(
                        _graphics->GetD3DDevice(),
                        _hwnd,
                        &swapChainDesc,
                        &fsSwapChainDesc,
                        nullptr,
                        &_swapChain
                    ));

                ThrowIfFailed(_graphics->GetDXGIFactory()->MakeWindowAssociation(_hwnd, DXGI_MWA_NO_ALT_ENTER));
            }
            else
            {
                ThrowIfFailed(_graphics->GetDXGIFactory()->CreateSwapChainForCoreWindow(
                    _graphics->GetD3DDevice(),
                    _window,
                    &swapChainDesc,
                    nullptr,
                    &_swapChain
                ));

                // Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
                // ensures that the application will only render after each VSync, minimizing power consumption.
                ComPtr<IDXGIDevice3> dxgiDevice;
                ThrowIfFailed(_graphics->GetD3DDevice()->QueryInterface(IID_PPV_ARGS(&dxgiDevice)));
                ThrowIfFailed(dxgiDevice->SetMaximumFrameLatency(1));

                // TODO: Handle rotation.
                ThrowIfFailed(_swapChain->SetRotation(DXGI_MODE_ROTATION_IDENTITY));
            }
        }

        ID3D11Texture2D* d3dBackbufferTexture;
        ThrowIfFailed(_swapChain->GetBuffer(0, IID_PPV_ARGS(&d3dBackbufferTexture)));
        _backbufferTexture = new D3D11Texture(_graphics, d3dBackbufferTexture);

        RenderPassDescription passDescription = {};
        passDescription.colorAttachments[0].texture = _backbufferTexture;
        passDescription.colorAttachments[0].loadAction = LoadAction::Clear;
        passDescription.colorAttachments[0].storeAction = StoreAction::Store;
        _renderPass = new D3D11RenderPass(_graphics, passDescription);

        // Set new size.
        _width = width;
        _height = height;
    }

    void D3D11SwapChain::Present()
    {
        HRESULT hr;
        if (_allowTearing)
        {
            // Recommended to always use tearing if supported when using a sync interval of 0.
            hr = _swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
        }
        else
        {
            // The first argument instructs DXGI to block until VSync, putting the application
            // to sleep until the next VSync. This ensures we don't waste any cycles rendering
            // frames that will never be displayed to the screen.
            hr = _swapChain->Present(1, 0);
        }


        //m_d3dContext->DiscardView(m_d3dRenderTargetView.Get());

        // If the device was removed either by a disconnection or a driver upgrade, we 
        // must recreate all device resources.
        if (hr == DXGI_ERROR_DEVICE_REMOVED
            || hr == DXGI_ERROR_DEVICE_RESET)
        {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n", (hr == DXGI_ERROR_DEVICE_REMOVED) ? _graphics->GetD3DDevice()->GetDeviceRemovedReason() : hr);
            OutputDebugStringA(buff);
#endif

            _graphics->HandleDeviceLost();
        }
        else
        {
            ThrowIfFailed(hr);

            if (!_graphics->GetDXGIFactory()->IsCurrent())
            {
                // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
                //CreateFactory();
            }
        }
    }
}
