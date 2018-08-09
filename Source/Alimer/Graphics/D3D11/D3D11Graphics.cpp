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

#include "D3D11Graphics.h"
#include "D3D11SwapChain.h"
#include "D3D11RenderPass.h"
#include "D3D11Texture.h"
#include "D3D11CommandBuffer.h"
#include "D3D11GpuBuffer.h"
#include "D3D11Shader.h"
#include "D3D11PipelineState.h"
#include "D3D11GpuAdapter.h"
#include "../ShaderCompiler.h"
#include "../../Core/Platform.h"
#include "../../Core/Log.h"
#include "../../Application/Windows/WindowWindows.h"
#include <STB/stb_image_write.h>

using namespace Microsoft::WRL;

namespace Alimer
{
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

    bool D3D11Graphics::IsSupported()
    {
        return true;
    }

    D3D11Graphics::D3D11Graphics(bool validation)
        : Graphics(GraphicsDeviceType::Direct3D11, validation)
    {
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
        if (FAILED(hr))
        {
            return;
        }

        // Enumerate adapters.
        ComPtr<IDXGIAdapter1> adapter;
        for (UINT adapterIndex = 0;
            _dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            _adapters.push_back(new D3D11GpuAdapter(adapter));
        }
    }

    D3D11Graphics::~D3D11Graphics()
    {
        Finalize();
    }

    void D3D11Graphics::Finalize()
    {
        WaitIdle();

        SafeDelete(_swapChain);
        _defaultCommandBuffer.Reset();

        Graphics::Finalize();
    }

    bool D3D11Graphics::BackendInitialize()
    {
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
        if (_validation)
        {
            if (SdkLayersAvailable())
            {
                // If the project is in a debug build, enable debugging via SDK Layers with this flag.
                creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
            }
            else
            {
                OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
            }
        }
#endif

        static const D3D_FEATURE_LEVEL s_featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        // Create the Direct3D 11 API device object and a corresponding context.
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;

        HRESULT hr = E_FAIL;
        hr = D3D11CreateDevice(
            static_cast<D3D11GpuAdapter*>(_adapter)->GetDXGIAdapter(),
            D3D_DRIVER_TYPE_UNKNOWN,
            0,
            creationFlags,
            s_featureLevels,
            _countof(s_featureLevels),
            D3D11_SDK_VERSION,
            device.GetAddressOf(),  // Returns the Direct3D device created.
            &_d3dFeatureLevel,     // Returns feature level of device created.
            context.GetAddressOf()  // Returns the device immediate context.
        );

        if (FAILED(hr))
        {
            // If the initialization fails, fall back to the WARP device.
            // For more information on WARP, see: 
            // http://go.microsoft.com/fwlink/?LinkId=286690
            hr = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
                0,
                creationFlags,
                s_featureLevels,
                _countof(s_featureLevels),
                D3D11_SDK_VERSION,
                device.GetAddressOf(),
                &_d3dFeatureLevel,
                context.GetAddressOf()
            );

            if (SUCCEEDED(hr))
            {
                ALIMER_LOGINFO("D3D11 backend - using WARP");
            }
        }

#if defined(_DEBUG)
        // https://blogs.msdn.microsoft.com/chuckw/2015/07/27/dxgi-debug-device/
#if ALIMER_PLATFORM_UWP
        Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
        {
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
        }
#else
        typedef HRESULT(WINAPI * LPDXGIGETDEBUGINTERFACE)(REFIID, void **);

        HMODULE dxgidebug = LoadLibraryExW(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (dxgidebug)
        {
            Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;

            auto dxgiGetDebugInterface = reinterpret_cast<LPDXGIGETDEBUGINTERFACE>(
                reinterpret_cast<void*>(GetProcAddress(dxgidebug, "DXGIGetDebugInterface")));

            if (SUCCEEDED(dxgiGetDebugInterface(IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
            {
                const GUID DXGI_DEBUG_ALL = { 0xe48ae283, 0xda80, 0x490b, { 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8 } };

                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
            }
        }
#endif

        ComPtr<ID3D11Debug> d3dDebug;
        if (SUCCEEDED(device.As(&d3dDebug)))
        {
            ComPtr<ID3D11InfoQueue> d3dInfoQueue;
            if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
            {
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

                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);

                //d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
            }
        }
#endif

        ThrowIfFailed(device.As(&_d3dDevice));
        ThrowIfFailed(context.As(&_d3dImmediateContext));
        ThrowIfFailed(context.As(&_d3dAnnotation));

        if (!InitializeCaps())
            return false;

        // Create Swapchain.
        const WindowHandle& handle = _window->getHandle();
        _swapChain = new D3D11SwapChain(this);

#if ALIMER_PLATFORM_UWP
        _swapChain->SetCoreWindow(static_cast<IUnknown*>(handle.handle), _window->getWidth(), _window->getHeight());
#else
        _swapChain->SetWindow(static_cast<HWND>(handle.handle), _window->getWidth(), _window->getHeight());
#endif

        // Immediate/default command queue.
        _defaultCommandBuffer = new D3D11CommandContext(this, _d3dImmediateContext.Get());

        return true;
    }

    bool D3D11Graphics::BeginFrame()
    {
        // TODO: Bind default viewport and render target.
        return true;
    }

    void D3D11Graphics::EndFrame()
    {
        // Present the frame.
        _swapChain->Present();

        // Flush immediate context.
        //_d3dContext->Flush();
    }

    SharedPtr<CommandBuffer> D3D11Graphics::RequestCommandBuffer(CommandBufferType type)
    {
        switch (type)
        {
        case CommandBufferType::Default:
            return _defaultCommandBuffer;

        default:
            ALIMER_LOGCRITICAL("Invalid CommandBuffer type requested: {}", str::ToString(type));
        }
    }

    void D3D11Graphics::Submit(const SharedPtr<CommandBuffer> &commandBuffer)
    {
        auto d3d11CommandBuffer = StaticCast<D3D11CommandBuffer>(commandBuffer);
        if (d3d11CommandBuffer->IsImmediate())
            return;

        // Execute deferred command buffer.
        ComPtr<ID3D11CommandList> commandList;
        ThrowIfFailed(_d3dImmediateContext->FinishCommandList(FALSE, commandList.ReleaseAndGetAddressOf()));
        _d3dImmediateContext->ExecuteCommandList(commandList.Get(), FALSE);
    }

    void D3D11Graphics::GenerateScreenshot(const std::string& fileName)
    {
        HRESULT hr = S_OK;

        D3D11Texture* backBufferTexture = _swapChain->GetBackbufferTexture();
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
            ALIMER_LOGCRITICAL("D3D11 - Failed to create texture, error: {}", std::to_string(hr));
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
                ALIMER_LOGCRITICAL("D3D11 - Failed to create texture, error: {}", std::to_string(hr));
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
            ALIMER_LOGCRITICAL("D3D11 - Failed to map resource, error: {}", std::to_string(hr));
        }

        if (!stbi_write_png(fileName.c_str(), textureDesc.Width, textureDesc.Height, 4, mappedSubresource.pData, static_cast<int>(mappedSubresource.RowPitch)))
        {
            _d3dImmediateContext->Unmap(texture, 0);
            texture->Release();
            ALIMER_LOGCRITICAL("D3D11 - Failed to save screenshot to file");
        }

        _d3dImmediateContext->Unmap(texture, 0);
        texture->Release();
    }

    void D3D11Graphics::WaitIdle()
    {
        _d3dImmediateContext->Flush();
    }

    bool D3D11Graphics::InitializeCaps()
    {
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

        D3D11_FEATURE_DATA_THREADING threadingSupport = { 0 };
        ThrowIfFailed(_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingSupport, sizeof(threadingSupport)));
        return true;
    }

    void D3D11Graphics::HandleDeviceLost()
    {
        // TODO
    }

    ID3D11InputLayout* D3D11Graphics::GetInputLayout(const InputLayoutDesc& desc)
    {
        // Check if layout already exists
        auto it = _inputLayouts.find(desc);
        if (it != end(_inputLayouts))
            return it->second;

        return nullptr;
    }

    void D3D11Graphics::StoreInputLayout(const InputLayoutDesc& desc, ID3D11InputLayout* layout)
    {
        ALIMER_ASSERT(_inputLayouts[desc] == nullptr);
        _inputLayouts[desc] = layout;
    }

    RenderPass* D3D11Graphics::GetBackbufferRenderPass() const
    {
        return _swapChain->GetRenderPass();
    }

    SharedPtr<RenderPass> D3D11Graphics::CreateRenderPass(const RenderPassDescription& description)
    {
        return MakeShared<D3D11RenderPass>(this, description);
    }

    BufferHandle* D3D11Graphics::CreateBuffer(BufferUsageFlags usage, uint64_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData)
    {
        return new D3D11GpuBuffer(this, usage, size, stride, resourceUsage, initialData);
    }

    SharedPtr<Texture> D3D11Graphics::CreateTexture(const TextureDescription& description, const ImageLevel* initialData)
    {
        return MakeShared<D3D11Texture>(this, description, initialData);
    }

    Shader* D3D11Graphics::CreateComputeShader(const void *pCode, size_t codeSize)
    {
        return new D3D11Shader(this, pCode, codeSize);
    }

    Shader* D3D11Graphics::CreateShader(
        const void *pVertexCode, size_t vertexCodeSize,
        const void *pFragmentCode, size_t fragmentCodeSize)
    {
        return new D3D11Shader(this,
            pVertexCode, vertexCodeSize,
            pFragmentCode, fragmentCodeSize
        );
    }

    PipelineState* D3D11Graphics::CreateRenderPipelineState(const RenderPipelineDescription& description)
    {
        return new D3D11PipelineState(this, description);
    }
}
