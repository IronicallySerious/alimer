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

#include "DeviceD3D11.h"
#include "SwapChainD3D11.h"
#include "CommandContextD3D11.h"
#include "TextureD3D11.h"
#include "SamplerD3D11.h"
#include "FramebufferD3D11.h"
#include "BufferD3D11.h"
#include "ShaderD3D11.h"
//#include "D3D11Pipeline.h"
#include "../../Core/Platform.h"
#include "../../Core/Log.h"
#include <STB/stb_image_write.h>

using namespace Microsoft::WRL;

#if ALIMER_D3D_DYNAMIC_LIB
typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY1)(REFIID riid, _COM_Outptr_ void **ppFactory);
typedef HRESULT(WINAPI* PFN_GET_DXGI_DEBUG_INTERFACE)(REFIID riid, _COM_Outptr_ void** pDebug);
#endif

namespace alimer
{
#ifdef ALIMER_D3D_DYNAMIC_LIB
    static HMODULE s_dxgiLib = nullptr;
    static HMODULE s_d3d11Lib = nullptr;

    static PFN_CREATE_DXGI_FACTORY1 CreateDXGIFactory1 = nullptr;
    static PFN_GET_DXGI_DEBUG_INTERFACE DXGIGetDebugInterface = nullptr;
    static PFN_D3D11_CREATE_DEVICE D3D11CreateDevice = nullptr;

    HRESULT D3D11LoadLibraries()
    {
        static bool loaded = false;
        if (loaded)
            return S_OK;
        loaded = true;

        // Load libraries first.
        s_dxgiLib = LoadLibraryW(L"dxgi.dll");
        if (!s_dxgiLib)
        {
            OutputDebugStringW(L"Failed to load dxgi.dll");
            return S_FALSE;
        }

        s_d3d11Lib = LoadLibraryW(L"d3d11.dll");
        if (!s_d3d11Lib)
        {
            OutputDebugStringW(L"Failed to load d3d11.dll");
            return S_FALSE;
        }

        /* DXGI entry points */
        CreateDXGIFactory1 = (PFN_CREATE_DXGI_FACTORY1)GetProcAddress(s_dxgiLib, "CreateDXGIFactory1");
        DXGIGetDebugInterface = (PFN_GET_DXGI_DEBUG_INTERFACE)GetProcAddress(s_dxgiLib, "DXGIGetDebugInterface");
        if (CreateDXGIFactory1 == nullptr)
        {
            OutputDebugStringW(L"Cannot find CreateDXGIFactory1 entry point.");
            return S_FALSE;
        }

        /* D3D11 entry points */
        D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(s_d3d11Lib, "D3D11CreateDevice");

        if (!D3D11CreateDevice)
        {
            OutputDebugStringW(L"Cannot find D3D11CreateDevice entry point.");
            return S_FALSE;
        }

        return S_OK;
    }
#endif

#if defined(_DEBUG)
    // Check for SDK Layer support.
    inline bool SdkLayersAvailable()
    {
        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
            0,
            D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
            nullptr,                    // Any feature level will do.
            0,
            D3D11_SDK_VERSION,
            nullptr,                    // No need to keep the D3D device reference.
            nullptr,                    // No need to know the feature level.
            nullptr                     // No need to keep the D3D device context reference.
        );

        return SUCCEEDED(hr);
    }
#endif

    static void GetD3D11HardwareAdapter(_In_ IDXGIFactory1* factory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
    {
        ComPtr<IDXGIAdapter1> adapter;
        *ppAdapter = nullptr;

        for (UINT adapterIndex = 0;
            factory->EnumAdapters1(adapterIndex, adapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND;
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            if (FAILED(adapter->GetDesc1(&desc)))
            {
                ALIMER_LOGERROR("DXGI - Failed to get desc");
            }

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                continue;
            }

            break;
        }

        *ppAdapter = adapter.Detach();
    }


    bool DeviceD3D11::IsSupported()
    {
        static bool availableCheck = false;
        static bool isAvailable = false;

        if (availableCheck)
            return isAvailable;

        availableCheck = true;
#if ALIMER_D3D_DYNAMIC_LIB
        if (FAILED(D3D11LoadLibraries()))
        {
            isAvailable = false;
            return false;
        }
#endif
        // Create temp dxgi factory for check support.
        ComPtr<IDXGIFactory1> factory;
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            isAvailable = false;
            return false;
        }

        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetD3D11HardwareAdapter(factory.Get(), &hardwareAdapter);
        isAvailable = hardwareAdapter != nullptr;
        return isAvailable;
    }

    DeviceD3D11::DeviceD3D11(bool validation)
        : GPUDevice(GraphicsBackend::D3D11, validation)
        , _cache(this)
    {
        if (FAILED(D3D11LoadLibraries()))
        {
            ALIMER_LOGCRITICAL("D3D11 - Failed to load functions");
            return;
        }

        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&_factory));
        if (FAILED(hr))
        {
            ALIMER_LOGCRITICAL("D3D11 - Failed to create DXGIFactory1");
            return;
        }

        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
        if (validation)
        {
            if (SdkLayersAvailable())
            {
                // If the project is in a debug build, enable debugging via SDK Layers with this flag.
                creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
            }
            else
            {
                ALIMER_LOGDEBUG("Direct3D Debug Device is not available");
            }
        }
#endif
        // Determine DirectX hardware feature levels this app will support.
        static const D3D_FEATURE_LEVEL s_featureLevels[] =
        {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        GetD3D11HardwareAdapter(_factory.Get(), _adapter.ReleaseAndGetAddressOf());
        UINT featLevelCount = _countof(s_featureLevels);
        hr = D3D11CreateDevice(
            _adapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            0,
            creationFlags,
            s_featureLevels,
            featLevelCount,
            D3D11_SDK_VERSION,
            _d3dDevice.ReleaseAndGetAddressOf(),
            &_d3dFeatureLevel,
            _d3dContext.ReleaseAndGetAddressOf()
        );

        if (hr == E_INVALIDARG && featLevelCount > 1)
        {
            assert(s_featureLevels[0] == D3D_FEATURE_LEVEL_11_1);

            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice(
                _adapter.Get(),
                D3D_DRIVER_TYPE_UNKNOWN,
                nullptr,
                creationFlags,
                &s_featureLevels[1],
                featLevelCount - 1,
                D3D11_SDK_VERSION,
                _d3dDevice.ReleaseAndGetAddressOf(),
                &_d3dFeatureLevel,
                _d3dContext.ReleaseAndGetAddressOf()
            );
        }
        ThrowIfFailed(hr);

#ifndef NDEBUG
        ComPtr<ID3D11Debug> d3dDebug;
        if (SUCCEEDED(_d3dDevice.As(&d3dDebug)))
        {
            ComPtr<ID3D11InfoQueue> d3dInfoQueue;
            if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
            {
#ifdef _DEBUG
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
                D3D11_INFO_QUEUE_FILTER filter;
                memset(&filter, 0, sizeof(filter));

                D3D11_MESSAGE_SEVERITY denySeverity = D3D11_MESSAGE_SEVERITY_INFO;
                filter.DenyList.NumSeverities = 1;
                filter.DenyList.pSeverityList = &denySeverity;

                D3D11_MESSAGE_ID denyIds[] =
                {
                    D3D11_MESSAGE_ID_OMSETRENDERTARGETS_INVALIDVIEW,
                    D3D11_MESSAGE_ID_DEVICE_DRAW_INDEX_BUFFER_TOO_SMALL,
                    D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
                    D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                };

                filter.DenyList.NumIDs = sizeof(denyIds) / sizeof(D3D11_MESSAGE_ID);
                filter.DenyList.pIDList = (D3D11_MESSAGE_ID*)&denyIds;
                d3dInfoQueue->PushStorageFilter(&filter);
            }
        }
#endif /* NDEBUG */

        // Obtain Direct3D 11.1 interfaces (if available)
        if (SUCCEEDED(_d3dDevice.As(&_d3dDevice1)))
        {
        }

        InitializeCaps();

        // Create immediate command buffer.
        _immediateCommandContext = new CommandContextD3D11(this);
    }

    DeviceD3D11::~DeviceD3D11()
    {
        _cache.Clear();

        _d3dContext.Reset();
        _d3dDevice1.Reset();

#ifdef _DEBUG
        {
            ComPtr<ID3D11Debug> d3dDebug;
            if (SUCCEEDED(_d3dDevice.As(&d3dDebug)))
            {
                d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
            }
        }
#endif
        _d3dDevice.Reset();
    }

    void DeviceD3D11::GenerateScreenshot(const std::string& fileName)
    {
        /*HRESULT hr = S_OK;

        TextureD3D11* backBufferTexture = _swapChain->GetBackbufferTexture();
        const DXGI_FORMAT format = backBufferTexture->GetDXGIFormat();

        D3D11_TEXTURE2D_DESC textureDesc;
        textureDesc.Width = backBufferTexture->GetWidth();
        textureDesc.Height = backBufferTexture->GetHeight();
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = format;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_STAGING;
        textureDesc.BindFlags = 0;
        textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        textureDesc.MiscFlags = 0;

        ID3D11Texture2D* texture;
        hr = _d3dDevice->CreateTexture2D(&textureDesc, nullptr, &texture);

        if (FAILED(hr))
        {
            ALIMER_LOGCRITICALF("D3D11 - Failed to create texture, error: %08X", static_cast<uint32_t>(hr));
        }

        // Resolve multisample texture.
        if (backBufferTexture->GetSamples() > SampleCount::Count1)
        {
            D3D11_TEXTURE2D_DESC resolveTextureDesc;
            resolveTextureDesc.Width = backBufferTexture->GetWidth();
            resolveTextureDesc.Height = backBufferTexture->GetHeight();
            resolveTextureDesc.MipLevels = 1;
            resolveTextureDesc.ArraySize = 1;
            resolveTextureDesc.Format = format;
            resolveTextureDesc.SampleDesc.Count = 1;
            resolveTextureDesc.SampleDesc.Quality = 0;
            resolveTextureDesc.Usage = D3D11_USAGE_DEFAULT;
            resolveTextureDesc.BindFlags = 0;
            resolveTextureDesc.CPUAccessFlags = 0;
            resolveTextureDesc.MiscFlags = 0;

            ID3D11Texture2D* resolveTexture;
            hr = _d3dDevice->CreateTexture2D(&resolveTextureDesc, nullptr, &resolveTexture);

            if (FAILED(hr))
            {
                texture->Release();
                ALIMER_LOGCRITICALF("D3D11 - Failed to create texture, error: %08X", static_cast<uint32_t>(hr));
            }

            _d3dImmediateContext->ResolveSubresource(resolveTexture, 0, backBufferTexture->GetResource(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
            _d3dImmediateContext->CopyResource(texture, resolveTexture);
            resolveTexture->Release();
        }
        else
        {
            _d3dImmediateContext->CopyResource(texture, backBufferTexture->GetResource());
        }

        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        hr = _d3dImmediateContext->Map(texture, 0, D3D11_MAP_READ, 0, &mappedSubresource);
        if (FAILED(hr))
        {
            texture->Release();
            ALIMER_LOGCRITICALF("D3D11 - Failed to map resource, error: %08X", static_cast<uint32_t>(hr));
        }

        if (!stbi_write_png(fileName.c_str(), textureDesc.Width, textureDesc.Height, 4, mappedSubresource.pData, static_cast<int>(mappedSubresource.RowPitch)))
        {
            _d3dImmediateContext->Unmap(texture, 0);
            texture->Release();
            ALIMER_LOGCRITICAL("D3D11 - Failed to save screenshot to file");
        }

        _d3dImmediateContext->Unmap(texture, 0);
        texture->Release();*/
    }

    bool DeviceD3D11::WaitIdle()
    {
        _d3dContext->Flush();
        return true;
    }

    void DeviceD3D11::InitializeCaps()
    {
        ComPtr<IDXGIDevice1> dxgiDevice;
        ThrowIfFailed(_d3dDevice.As(&dxgiDevice));

        ComPtr<IDXGIAdapter> dxgiAdapter;
        ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        DXGI_ADAPTER_DESC desc;
        dxgiAdapter->GetDesc(&desc);

#ifdef _DEBUG
        wchar_t buff[256] = {};
        swprintf_s(buff, L"D3D11 device created using Adapter: VID:%04X, PID:%04X - %ls\n", desc.VendorId, desc.DeviceId, desc.Description);
        OutputDebugStringW(buff);
#endif
        _features.SetVendorId(desc.VendorId);
        _features.SetDeviceId(desc.DeviceId);
        _features.SetDeviceName(String(desc.Description));

        switch (_d3dFeatureLevel)
        {
        case D3D_FEATURE_LEVEL_10_0:
            _shaderModelMajor = 4;
            _shaderModelMinor = 0;
            break;
        case D3D_FEATURE_LEVEL_10_1:
            _shaderModelMajor = 4;
            _shaderModelMinor = 1;
            break;
        case D3D_FEATURE_LEVEL_11_0:
            _shaderModelMajor = 5;
            _shaderModelMinor = 0;
            break;
        case D3D_FEATURE_LEVEL_11_1:
            _shaderModelMajor = 5;
            _shaderModelMinor = 0;
            break;
        case D3D_FEATURE_LEVEL_12_0:
            _shaderModelMajor = 5;
            _shaderModelMinor = 1;
            break;
        case D3D_FEATURE_LEVEL_12_1:
            _shaderModelMajor = 5;
            _shaderModelMinor = 1;
            break;
        default:
            break;
        }

        D3D11_FEATURE_DATA_THREADING threadingFeature = { 0 };
        HRESULT hr = _d3dDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingFeature, sizeof(threadingFeature));
        if (SUCCEEDED(hr)
            && threadingFeature.DriverConcurrentCreates
            && threadingFeature.DriverCommandLists)
        {
            _features.SetMultithreading(true);
        }

        _limits.maxColorAttachments = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
        _limits.maxBindGroups = MaxDescriptorSets;
        _limits.minUniformBufferOffsetAlignment = 16;

        IDXGIFactory5* factory5;
        if (SUCCEEDED(_factory->QueryInterface(&factory5)))
        {
            BOOL allowTearing = FALSE;
            hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
            if (SUCCEEDED(hr) && allowTearing)
            {
                _allowTearing = true;
            }
        }
    }

    void DeviceD3D11::HandleDeviceLost()
    {
        // TODO
    }

    D3D11Cache &DeviceD3D11::GetCache()
    {
        return _cache;
    }

    SwapChain* DeviceD3D11::CreateSwapChainImpl(const SwapChainDescriptor* descriptor)
    {
        return new SwapChainD3D11(this, descriptor, 2);
    }

    Texture* DeviceD3D11::CreateTextureImpl(const TextureDescriptor* descriptor, const void* initialData)
    {
        return new TextureD3D11(this, descriptor, initialData, nullptr, DXGI_FORMAT_UNKNOWN);
    }

    Framebuffer* DeviceD3D11::CreateFramebufferImpl(const FramebufferDescriptor* descriptor)
    {
        return new FramebufferD3D11(this, descriptor);
    }

    Buffer* DeviceD3D11::CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData)
    {
        return new BufferD3D11(this, descriptor, initialData);
    }

    Sampler* DeviceD3D11::CreateSamplerImpl(const SamplerDescriptor* descriptor)
    {
        return new SamplerD3D11(this, descriptor);
    }

    Shader* DeviceD3D11::CreateShaderImpl(const ShaderDescriptor* descriptor)
    {
        return new ShaderD3D11(this, descriptor);
    }
}
