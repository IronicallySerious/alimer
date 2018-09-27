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
#include "D3D11GraphicsDevice.h"
#include "D3D11Texture.h"
#include "D3D11RenderPass.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11SwapChain::D3D11SwapChain(D3D11Graphics* graphics)
        : _graphics(graphics)
        , _allowTearing(graphics->GetAllowTearing())
    {
    }

    D3D11SwapChain::~D3D11SwapChain()
    {
        _swapChain->SetFullscreenState(false, nullptr);
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
        _graphics->GetD3DImmediateContext()->OMSetRenderTargets(1, nullViews, nullptr);

        HRESULT hr = S_OK;

        if (_swapChain)
        {
            hr = _swapChain->ResizeBuffers(
                _backBufferCount,
                width,
                height,
                _backBufferFormat,
                _allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0
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
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = _backBufferCount;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = _allowTearing ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags = _allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

            if (!_window)
            {
                DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
                fsSwapChainDesc.Windowed = TRUE;

                hr = _graphics->GetDXGIFactory()->CreateSwapChainForHwnd(
                    _graphics->GetD3DDevice(),
                    _hwnd,
                    &swapChainDesc,
                    &fsSwapChainDesc,
                    nullptr,
                    &_swapChain
                );

                if (FAILED(hr))
                {
                    ALIMER_LOGCRITICAL("Failed to create DXGI SwapChain");
                }

                ThrowIfFailed(_graphics->GetDXGIFactory()->MakeWindowAssociation(_hwnd, DXGI_MWA_NO_ALT_ENTER));
            }
            else
            {
                swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

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

        D3D11_TEXTURE2D_DESC d3dTextureDesc;
        d3dBackbufferTexture->GetDesc(&d3dTextureDesc);

        TextureDescriptor textureDesc = {};
        textureDesc.type = TextureType::Type2D;
        textureDesc.format = d3d::Convert(d3dTextureDesc.Format);
        textureDesc.width = d3dTextureDesc.Width;
        textureDesc.height = d3dTextureDesc.Height;
        textureDesc.depth = 1;
        if (d3dTextureDesc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
        {
            textureDesc.type = TextureType::TypeCube;
            textureDesc.arrayLayers = d3dTextureDesc.ArraySize / 6;
        }
        else
        {
            textureDesc.arrayLayers = d3dTextureDesc.ArraySize;
        }
        textureDesc.mipLevels = d3dTextureDesc.MipLevels;
        textureDesc.usage = TextureUsage::Unknown;
        if (d3dTextureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
        {
            textureDesc.usage |= TextureUsage::ShaderRead;
        }

        if (d3dTextureDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
        {
            textureDesc.usage |= TextureUsage::ShaderWrite;
        }

        if (d3dTextureDesc.BindFlags & D3D11_BIND_RENDER_TARGET
            || d3dTextureDesc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
        {
            textureDesc.usage |= TextureUsage::RenderTarget;
        }

        _backbufferTexture = new D3D11Texture(_graphics, &textureDesc, nullptr, d3dBackbufferTexture);

        RenderPassDescription passDescription = {};
        passDescription.colorAttachments[0].texture = _backbufferTexture;
        passDescription.colorAttachments[0].loadAction = LoadAction::Clear;
        passDescription.colorAttachments[0].storeAction = StoreAction::Store;
        _renderPass = new D3D11RenderPass(_graphics, &passDescription);

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
