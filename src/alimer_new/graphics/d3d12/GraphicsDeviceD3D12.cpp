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

#include "GraphicsDeviceD3D12.h"
#include "GraphicsDeviceFactoryD3D12.h"
#include "SwapChainD3D12.h"

namespace alimer
{
    GraphicsDeviceD3D12::GraphicsDeviceD3D12(GraphicsDeviceFactoryD3D12* factory, ComPtr<IDXGIAdapter1> adapter)
        : _factory(factory)
        , _adapter(adapter)
    {
        DXGI_ADAPTER_DESC1 desc = { };
        adapter->GetDesc1(&desc);
        //WriteLog("Creating DX12 device on adapter '%ls'", desc.Description);

        ThrowIfFailed(D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device)));

        // Check the maximum feature level, and make sure it's above our minimum
        D3D_FEATURE_LEVEL featureLevelsArray[4];
        featureLevelsArray[0] = D3D_FEATURE_LEVEL_11_0;
        featureLevelsArray[1] = D3D_FEATURE_LEVEL_11_1;
        featureLevelsArray[2] = D3D_FEATURE_LEVEL_12_0;
        featureLevelsArray[3] = D3D_FEATURE_LEVEL_12_1;
        D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevels = { };
        featureLevels.NumFeatureLevels = _countof(featureLevelsArray);
        featureLevels.pFeatureLevelsRequested = featureLevelsArray;
        ThrowIfFailed(_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevels, sizeof(featureLevels)));
        _featureLevel = featureLevels.MaxSupportedFeatureLevel;

        const D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0;
        if (_featureLevel < minFeatureLevel)
        {
            // TODO: Exception
        }

#if D3D12_DEBUG
        if (_factory->Validation())
        {
            // Configure debug device (if active).
            ComPtr<ID3D12InfoQueue> infoQueue;
            if (SUCCEEDED(_device.As(&infoQueue)))
            {
                D3D12_MESSAGE_ID hide[] =
                {
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,

                    // These happen when capturing with VS diagnostics
                    D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
                };
                D3D12_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = _countof(hide);
                filter.DenyList.pIDList = hide;
                infoQueue->AddStorageFilterEntries(&filter);

                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                //infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
            }
        }
#endif

        InitializeCaps();
    }

    GraphicsDeviceD3D12::~GraphicsDeviceD3D12()
    {
    }

    SwapChain* GraphicsDeviceD3D12::CreateSwapChainImpl(SwapChainSurface* surface, const SwapChainDescriptor* descriptor)
    {
        return new SwapChainD3D12(surface, descriptor);
    }

    void GraphicsDeviceD3D12::InitializeCaps()
    {
    }
}
