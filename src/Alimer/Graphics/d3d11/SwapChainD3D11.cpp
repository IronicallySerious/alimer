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

#include "SwapChainD3D11.h"
#include "GraphicsDeviceD3D11.h"
#include "TextureD3D11.h"
#include "FramebufferD3D11.h"
#include "D3D11Convert.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace alimer
{
    SwapChainD3D11::SwapChainD3D11(GraphicsDeviceD3D11* device, const SwapChainDescriptor* descriptor)
        : Window(descriptor->title, descriptor->width, descriptor->height, descriptor->resizable, descriptor->fullscreen)
        , _device(device)
        , _handle(nullptr)
    {
        _backBufferFormat = PixelFormat::BGRA8UNorm;
        _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

        if (descriptor->srgb)
        {
            _backBufferFormat = PixelFormat::BGRA8UNormSrgb;
            _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        }

        if (IsOpen()) {
            OnHandleCreated();
        }
    }

    SwapChainD3D11::~SwapChainD3D11()
    {
        Destroy();
    }

    void SwapChainD3D11::Destroy()
    {
        _swapChain->SetFullscreenState(false, nullptr);
        _swapChain.Reset();
        _swapChain1.Reset();
    }

    void SwapChainD3D11::OnHandleCreated()
    {
        _handle = GetNativeHandle();
        ResizeImpl(_size.x, _size.y);
    }

    void SwapChainD3D11::OnSizeChanged(const IntVector2& newSize)
    {
        ResizeImpl(newSize.x, newSize.y);
    }

    void SwapChainD3D11::ResizeImpl(uint32_t width, uint32_t height)
    {
        HRESULT hr = S_OK;

        if (_swapChain)
        {
            SafeDelete(_backbufferTexture);

            hr = _swapChain->ResizeBuffers(NumBackBuffers, width, height, DXGI_FORMAT_UNKNOWN, _swapChainFlags);

            if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
            {
                // If the device was removed for any reason, a new device and swap chain will need to be created.
                _device->HandleDeviceLost();

                return;
            }
        }
        else
        {
            // Check tearing.
            IDXGIFactory2* dxgiFactory2;
            if (SUCCEEDED(_device->GetFactory()->QueryInterface(&dxgiFactory2)))
            {
                DXGI_SWAP_EFFECT swapEffect = DXGI_SWAP_EFFECT_DISCARD;
                if (_device->AllowTearing())
                {
                    _syncInterval = 0;
                    _presentFlags = DXGI_PRESENT_ALLOW_TEARING;
                    _swapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
                    // Cannot use srgb format with flip swap effect.
                    swapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                    _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
                }

                // DirectX 11.1 or later.
                DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
                swapChainDesc.Width = width;
                swapChainDesc.Height = height;
                swapChainDesc.Format = _dxgiBackBufferFormat;
                swapChainDesc.SampleDesc.Count = 1;
                swapChainDesc.SampleDesc.Quality = 0;
                swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
                swapChainDesc.BufferCount = NumBackBuffers;
                swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
                swapChainDesc.SwapEffect = swapEffect;
                swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
                swapChainDesc.Flags = _swapChainFlags;

#if !ALIMER_PLATFORM_UWP
                DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
                fsSwapChainDesc.Windowed = TRUE;

                hr = dxgiFactory2->CreateSwapChainForHwnd(
                    _device->GetD3DDevice(),
                    _handle,
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
#else
                swapChainDesc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH;
                swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

                ThrowIfFailed(dxgiFactory2->CreateSwapChainForCoreWindow(
                    _device->GetD3DDevice(),
                    _handle,
                    &swapChainDesc,
                    nullptr,
                    &_swapChain1
                ));

                // Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
                // ensures that the application will only render after each VSync, minimizing power consumption.
                //ComPtr<IDXGIDevice3> dxgiDevice3;
                //ThrowIfFailed(_device->GetD3DDevice()->QueryInterface(IID_PPV_ARGS(&dxgiDevice3)));
                //ThrowIfFailed(dxgiDevice3->SetMaximumFrameLatency(1));

                // TODO: Handle rotation.
                ThrowIfFailed(_swapChain1->SetRotation(DXGI_MODE_ROTATION_IDENTITY));
#endif

                dxgiFactory2->Release();
            }
            else
            {
                // DirectX 11.0
                DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
                swapChainDesc.BufferDesc.Width = width;
                swapChainDesc.BufferDesc.Height = height;
                swapChainDesc.BufferDesc.Format = _dxgiBackBufferFormat;
                swapChainDesc.SampleDesc.Count = 1;
                swapChainDesc.SampleDesc.Quality = 0;
                swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
                swapChainDesc.BufferCount = NumBackBuffers;
                swapChainDesc.OutputWindow = _handle;
                swapChainDesc.Windowed = TRUE;
                swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

                ThrowIfFailed(_device->GetFactory()->CreateSwapChain(
                    _device->GetD3DDevice(),
                    &swapChainDesc,
                    _swapChain.ReleaseAndGetAddressOf()
                ));
            }
        }

#if !ALIMER_PLATFORM_UWP
        ThrowIfFailed(_device->GetFactory()->MakeWindowAssociation(_handle, DXGI_MWA_NO_ALT_ENTER));
#endif

        ID3D11Texture2D* renderTarget;
        ThrowIfFailed(_swapChain->GetBuffer(0, IID_PPV_ARGS(&renderTarget)));

        // Get D3D Texture desc and convert to engine.
        D3D11_TEXTURE2D_DESC textureDesc;
        renderTarget->GetDesc(&textureDesc);
        TextureDescriptor descriptor = d3d11::Convert(textureDesc);

        _backbufferTexture = new TextureD3D11(_device, &descriptor, renderTarget, nullptr);
        //InitializeFramebuffer();
    }

    void SwapChainD3D11::SwapBuffers()
    {
        HRESULT hr = _swapChain->Present(_syncInterval, _presentFlags);
        //m_d3dContext->DiscardView(m_d3dRenderTargetView.Get());

        // If the device was removed either by a disconnection or a driver upgrade, we 
        // must recreate all device resources.
        if (hr == DXGI_ERROR_DEVICE_REMOVED
            || hr == DXGI_ERROR_DEVICE_RESET)
        {
            _device->HandleDeviceLost();
        }
        else
        {
            ThrowIfFailed(hr);
        }
    }
}
