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
#include "D3D12Texture.h"
#include "../../Debug/Log.h"

namespace Alimer
{
    D3D12Swapchain::D3D12Swapchain(D3D12Graphics* graphics, const SwapchainDescriptor* descriptor)
        : _graphics(graphics)
    {
        // Create a descriptor for the swap chain.
        uint32_t backbufferCount = 2;
        if (descriptor->preferredColorFormat != PixelFormat::Unknown)
        {
            switch (descriptor->preferredColorFormat)
            {
            case PixelFormat::BGRA8UNormSrgb:
                _backBufferFormat = PixelFormat::BGRA8UNormSrgb;
                _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
                break;

            default:
                _backBufferFormat = PixelFormat::BGRA8UNorm;
                _dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
                break;
            }
        }

        if (descriptor->bufferCount != 0)
        {
            backbufferCount = descriptor->bufferCount;
            if (backbufferCount > 3)
                backbufferCount = 3;
        }

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = descriptor->width;
        swapChainDesc.Height = descriptor->height;
        swapChainDesc.Format = _dxgiBackBufferFormat;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backbufferCount;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapChainDesc.Flags =
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH |
            DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

        /* TODO: DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT*/

#if !ALIMER_PLATFORM_UWP
        HWND windowHandle = static_cast<HWND>(descriptor->windowHandle);
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a swap chain for the window.
        ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(_graphics->GetFactory()->CreateSwapChainForHwnd(
            _graphics->GetCommandListManager()->GetD3DCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
            windowHandle,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            &swapChain
        ));

        ThrowIfFailed(swapChain.As(&_swapChain));

        // This sample does not support fullscreen transitions.
        ThrowIfFailed(_graphics->GetFactory()->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER));
#else
        ThrowIfFailed(_graphics->GetFactory()->CreateSwapChainForCoreWindow(
            _graphics->GetCommandListManager()->GetD3DCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
            reinterpret_cast<IUnknown*>(Windows::UI::Core::CoreWindow::GetForCurrentThread()),
            &swapChainDesc,
            nullptr,
            &swapChain
        ));

        ThrowIfFailed(swapChain.As(&_swapChain));
#endif

        AfterReset();
    }

    void D3D12Swapchain::AfterReset()
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
            _backBufferTextures[i].Reset(new D3D12Texture(_graphics, resource));
            _backBufferTextures[i]->SetUsageState(D3D12_RESOURCE_STATE_PRESENT);
        }

        _backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
    }

    void D3D12Swapchain::Present()
    {
        const bool vsync = true;
        uint32_t syncIntervals = vsync ? 1 : 0;
        ThrowIfFailed(_swapChain->Present(syncIntervals, syncIntervals == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0));

        _backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
    }
}
