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

#include "SwapChainD3D11.h"
#include "DeviceD3D11.h"
#include "TextureD3D11.h"
#include "FramebufferD3D11.h"
#include "D3D11Convert.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    SwapChainD3D11::SwapChainD3D11(DeviceD3D11* device, void* window, uint32_t width, uint32_t height, PixelFormat depthStencilFormat, bool srgb, uint32_t backBufferCount)
        : _device(device)
        , _sRGB(srgb)
        , _backBufferCount(backBufferCount)
    {
        _depthStencilFormat = depthStencilFormat;

#if ALIMER_PLATFORM_UWP
        _window = static_cast<IUnknown*>(window);
#else
        _hwnd = static_cast<HWND>(window);
#endif

        Resize(width, height, true);
    }

    SwapChainD3D11::~SwapChainD3D11()
    {
        Destroy();
    }

    void SwapChainD3D11::Destroy()
    {
        _swapChain->SetFullscreenState(false, nullptr);
        _renderTarget.Reset();
        _swapChain.Reset();
        _swapChain1.Reset();
    }


    void SwapChainD3D11::Resize(uint32_t width, uint32_t height, bool force)
    {
        if (_width == width && _height == height && !force)
        {
            return;
        }

        HRESULT hr = S_OK;

        if (_swapChain)
        {
            //RenderWindow::Destroy();
            _renderTarget.Reset();

            hr = _swapChain->ResizeBuffers(_backBufferCount, width, height,
                _backBufferFormat, _swapChainFlags
            );

            if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
            {
            }
        }
        else
        {
            // Check tearing.
            _backBufferFormat = _sRGB ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;

            ComPtr<IDXGIFactory2> dxgiFactory2;
            if (SUCCEEDED(_device->GetFactory()->QueryInterface(dxgiFactory2.ReleaseAndGetAddressOf())))
            {
                DXGI_SWAP_EFFECT swapEffect = DXGI_SWAP_EFFECT_DISCARD;
                if (_device->AllowTearing())
                {
                    _syncInterval = 0;
                    _presentFlags = DXGI_PRESENT_ALLOW_TEARING;
                    _swapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
                    // Cannot use srgb format with flip swap effect.
                    swapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                    _backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
                }

                // DirectX 11.1 or later.
                DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
                swapChainDesc.Width = width;
                swapChainDesc.Height = height;
                swapChainDesc.Format = _backBufferFormat;
                swapChainDesc.SampleDesc.Count = 1;
                swapChainDesc.SampleDesc.Quality = 0;
                swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
                swapChainDesc.BufferCount = _backBufferCount;
                swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
                swapChainDesc.SwapEffect = swapEffect;
                swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
                swapChainDesc.Flags = _swapChainFlags;

                if (!_window)
                {
                    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
                    fsSwapChainDesc.Windowed = TRUE;

                    hr = dxgiFactory2->CreateSwapChainForHwnd(
                        _device->GetD3DDevice(),
                        _hwnd,
                        &swapChainDesc,
                        &fsSwapChainDesc,
                        nullptr,
                        _swapChain1.ReleaseAndGetAddressOf()
                    );

                    if (FAILED(hr))
                    {
                        ALIMER_LOGCRITICAL("Failed to create DXGI SwapChain, HRESULT {}", static_cast<unsigned int>(hr));
                    }

                    ThrowIfFailed(_swapChain1.As(&_swapChain));

                }
                else
                {
                    swapChainDesc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH;
                    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

                    ThrowIfFailed(dxgiFactory2->CreateSwapChainForCoreWindow(
                        _device->GetD3DDevice(),
                        _window,
                        &swapChainDesc,
                        nullptr,
                        &_swapChain1
                    ));

                    // Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
                    // ensures that the application will only render after each VSync, minimizing power consumption.
                    ComPtr<IDXGIDevice3> dxgiDevice3;
                    ThrowIfFailed(_device->GetD3DDevice()->QueryInterface(IID_PPV_ARGS(&dxgiDevice3)));
                    ThrowIfFailed(dxgiDevice3->SetMaximumFrameLatency(1));

                    // TODO: Handle rotation.
                    ThrowIfFailed(_swapChain1->SetRotation(DXGI_MODE_ROTATION_IDENTITY));
                }
            }
            else
            {
                // DirectX 11.0
                DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
                swapChainDesc.BufferDesc.Width = width;
                swapChainDesc.BufferDesc.Height = height;
                swapChainDesc.BufferDesc.Format = _backBufferFormat;
                swapChainDesc.SampleDesc.Count = 1;
                swapChainDesc.SampleDesc.Quality = 0;
                swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
                swapChainDesc.BufferCount = _backBufferCount;
                swapChainDesc.OutputWindow = _hwnd;
                swapChainDesc.Windowed = TRUE;
                swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

                ThrowIfFailed(_device->GetFactory()->CreateSwapChain(
                    _device->GetD3DDevice(),
                    &swapChainDesc,
                    _swapChain.ReleaseAndGetAddressOf()
                ));
            }
        }

        if (_hwnd)
        {
            ThrowIfFailed(_device->GetFactory()->MakeWindowAssociation(_hwnd, DXGI_MWA_NO_ALT_ENTER));
        }

        ID3D11Texture2D* renderTarget;
        ThrowIfFailed(_swapChain->GetBuffer(0, IID_PPV_ARGS(&renderTarget)));

        // Get D3D Texture desc and convert to engine.
        D3D11_TEXTURE2D_DESC textureDesc;
        renderTarget->GetDesc(&textureDesc);
        TextureDescriptor descriptor = d3d11::Convert(textureDesc);

        DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        if (_sRGB)
        {
            backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            descriptor.format = PixelFormat::BGRA8UNormSrgb;
        }

        _renderTarget = new TextureD3D11(_device, descriptor, nullptr, renderTarget, backBufferFormat);

        // Set new size.
        _width = width;
        _height = height;
    }

    void SwapChainD3D11::Configure(uint32_t width, uint32_t height)
    {
        Resize(width, height, true);
    }

    void SwapChainD3D11::Present()
    {
        HRESULT hr = _swapChain->Present(_syncInterval, _presentFlags);
        //m_d3dContext->DiscardView(m_d3dRenderTargetView.Get());

        // If the device was removed either by a disconnection or a driver upgrade, we 
        // must recreate all device resources.
        if (hr == DXGI_ERROR_DEVICE_REMOVED
            || hr == DXGI_ERROR_DEVICE_RESET)
        {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n", (hr == DXGI_ERROR_DEVICE_REMOVED) ? _device->GetD3DDevice()->GetDeviceRemovedReason() : hr);
            OutputDebugStringA(buff);
#endif

            _device->HandleDeviceLost();
        }
        else
        {
            ThrowIfFailed(hr);
        }
    }
}
