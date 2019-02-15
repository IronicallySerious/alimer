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

#include "D3D12Swapchain.h"
#include "D3D12Graphics.h"
#include "D3D12CommandListManager.h"
#include "D3D12Framebuffer.h"
#include "D3D12Texture.h"
#include "../D3D/D3DConvert.h"

using namespace Microsoft::WRL;

namespace alimer
{
    SwapChainD3D12::SwapChainD3D12(GraphicsDeviceD3D12* device, const SwapChainDescriptor* descriptor)
        : _device(device)
    {
        _backBufferFormat = PixelFormat::BGRA8UNorm;
        _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

        if (descriptor->srgb)
        {
            _backBufferFormat = PixelFormat::BGRA8UNormSrgb;
            _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        }

#if ALIMER_PLATFORM_UWP
        _window = reinterpret_cast<IUnknown*>(descriptor->nativeHandle);
#else
        _hwnd = reinterpret_cast<HWND>(descriptor->nativeHandle);
#endif

        Resize(descriptor->width, descriptor->height);
    }

    void SwapChainD3D12::Resize(uint32_t width, uint32_t height)
    {
        HRESULT hr = S_OK;

        if (_swapChain)
        {
            //SwapChain::Destroy();

            hr = _swapChain->ResizeBuffers(NumBackBuffers, width, height, DXGI_FORMAT_UNKNOWN, _swapChainFlags);

            if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
            {
            }
        }
        else
        {
            DXGI_SWAP_EFFECT swapEffect = DXGI_SWAP_EFFECT_DISCARD;
            if (_device->AllowTearing())
            {
                _syncInterval = 0;
                _presentFlags = DXGI_PRESENT_ALLOW_TEARING;
                _swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
                // Cannot use srgb format with flip swap effect.
                swapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
            }


            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.Width = width;
            swapChainDesc.Height = height;
            swapChainDesc.Format = _dxgiBackBufferFormat;
            swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = NumBackBuffers;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = swapEffect;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags = _swapChainFlags;

            /* TODO: DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT*/
            ComPtr<IDXGISwapChain1> swapChain;

            if (_hwnd)
            {
                DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
                fsSwapChainDesc.Windowed = TRUE;

                // Create a swap chain for the window.
                ThrowIfFailed(_device->GetFactory()->CreateSwapChainForHwnd(
                    _device->GetCommandListManager()->GetD3DCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
                    _hwnd,
                    &swapChainDesc,
                    &fsSwapChainDesc,
                    nullptr,
                    &swapChain
                ));

                

                // This sample does not support fullscreen transitions.
                ThrowIfFailed(_device->GetFactory()->MakeWindowAssociation(_hwnd, DXGI_MWA_NO_ALT_ENTER));
            }
#if ALIMER_PLATFORM_UWP
            else
            {
                ThrowIfFailed(_device->GetFactory()->CreateSwapChainForCoreWindow(
                    _device->GetCommandListManager()->GetD3DCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
                    _window,
                    &swapChainDesc,
                    nullptr,
                    &swapChain
                ));
            }
#endif

            ThrowIfFailed(swapChain->QueryInterface(&_swapChain));
            AfterReset();
        }
    }

    SwapChainD3D12::~SwapChainD3D12()
    {
        SafeRelease(_swapChain);
    }

    void SwapChainD3D12::AfterReset()
    {
        // Re-create an RTV for each back buffer
        for (uint32_t i = 0; i < NumBackBuffers; i++)
        {
            ID3D12Resource* resource;
            ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&resource)));

            wchar_t name[25] = {};
            swprintf_s(name, L"Back Buffer %u", i);
            resource->SetName(name);

            /* Create engine texture. */
            TextureD3D12* texture = new TextureD3D12(_device, resource);
            texture->SetUsageState(D3D12_RESOURCE_STATE_PRESENT);
            _backBufferTextures[i] = texture;

            /* Create framebuffer*/
            //FramebufferDescriptor framebufferDescriptor = {};
            //framebufferDescriptor.colorAttachments[0].texture = _backBufferTextures[i].Get();
            //_backBufferFramebuffers[i] = new Framebuffer();
            //_backBufferFramebuffers[i]->Define(&framebufferDescriptor);
        }

        _backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
    }

    void SwapChainD3D12::Present()
    {
        const bool vsync = true;
        uint32_t syncIntervals = vsync ? 1 : 0;
        HRESULT hr = _swapChain->Present(syncIntervals, syncIntervals == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0);
        if (FAILED(hr))
        {
            // TODO: Handle failure
        }
        _backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
    }
}
