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

#include "D3D12Graphics.h"
#include "D3D12Texture.h"
#include "D3D12CommandBuffer.h"
#include "../../Debug/Log.h"
#include "../../Core/Windows/WindowWindows.h"

namespace Alimer
{
#if !ALIMER_PLATFORM_UWP
	typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY2)(UINT flags, REFIID _riid, _Out_ void** ppFactory);
	typedef HRESULT(WINAPI* PFN_GET_DXGI_DEBUG_INTERFACE)(UINT flags, REFIID _riid, void** _debug);

	bool _d3d12_libInitialized = false;
	HMODULE _d3d12_DXGIDebugHandle = nullptr;
	HMODULE _d3d12_DXGIHandle = nullptr;
	HMODULE _d3d12_Handle = nullptr;

	PFN_GET_DXGI_DEBUG_INTERFACE DXGIGetDebugInterface1 = nullptr;
	PFN_CREATE_DXGI_FACTORY2 CreateDXGIFactory2 = nullptr;

	PFN_D3D12_CREATE_DEVICE D3D12CreateDevice = nullptr;
	PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterface = nullptr;
	PFN_D3D12_SERIALIZE_ROOT_SIGNATURE D3D12SerializeRootSignature = nullptr;
	static bool D3D12GetDebugInterfaceAvailable = false;

	HRESULT LoadD3D12Libraries()
	{
		if (_d3d12_libInitialized)
			return S_OK;

		_d3d12_DXGIDebugHandle = ::LoadLibraryW(L"dxgidebug.dll");
		_d3d12_DXGIHandle = ::LoadLibraryW(L"dxgi.dll");
		_d3d12_Handle = ::LoadLibraryW(L"d3d12.dll");

		if (!_d3d12_DXGIHandle)
			return E_FAIL;

		if (!_d3d12_Handle)
			return E_FAIL;

		// Load symbols.
		DXGIGetDebugInterface1 = (PFN_GET_DXGI_DEBUG_INTERFACE)::GetProcAddress(_d3d12_DXGIHandle, "DXGIGetDebugInterface1");
		CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)::GetProcAddress(_d3d12_DXGIHandle, "CreateDXGIFactory2");

		// We need at least D3D11.1
		if (!CreateDXGIFactory2)
			return E_FAIL;

		D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)::GetProcAddress(_d3d12_Handle, "D3D12CreateDevice");
		D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)::GetProcAddress(_d3d12_Handle, "D3D12GetDebugInterface");
		D3D12SerializeRootSignature = (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)::GetProcAddress(_d3d12_Handle, "D3D12SerializeRootSignature");

		if (!D3D12CreateDevice)
			return E_FAIL;

		if (!D3D12SerializeRootSignature)
			return E_FAIL;

		D3D12GetDebugInterfaceAvailable = D3D12GetDebugInterface != nullptr;

		_d3d12_libInitialized = true;
		return S_OK;
	}
#endif

	D3D12Graphics::D3D12Graphics()
	{

	}

	D3D12Graphics::~D3D12Graphics()
	{
		// Ensure that the GPU is no longer referencing resources that are about to be
		// cleaned up by the destructor.
		WaitIdle();

		CloseHandle(_fenceEvent);
	}

	bool D3D12Graphics::Initialize(std::shared_ptr<Window> window)
	{
#if !ALIMER_PLATFORM_UWP
		if (FAILED(LoadD3D12Libraries()))
		{
			//ALIMER_LOGERROR("Failed to load D3D12 libraries.");
			return false;
		}
#endif

		UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
		// Enable the debug layer (requires the Graphics Tools "optional feature").
		// NOTE: Enabling the debug layer after device creation will invalidate the active device.
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();

				// Enable additional debug layers.
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif

		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&_factory));
		if (FAILED(hr))
		{
			return false;
		}

		if (_useWarpDevice)
		{
			ThrowIfFailed(_factory->EnumWarpAdapter(IID_PPV_ARGS(&_adapter)));

			ThrowIfFailed(D3D12CreateDevice(
				_adapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&_d3dDevice)
			));
		}
		else
		{
			ComPtr<IDXGIAdapter1> adapter;
			for (UINT adapterIndex = 0;
				_factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see if the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}

			_adapter.Attach(adapter.Detach());

			ThrowIfFailed(D3D12CreateDevice(
				adapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&_d3dDevice)
			));
		}

		if (!InitializeCaps())
			return false;

#ifndef NDEBUG
		// Setup break on error + corruption.
		ComPtr<ID3D12InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(_d3dDevice.As(&d3dInfoQueue)))
		{
#ifdef _DEBUG
			d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif

			// Disable some warnings that we don't generally care about, unless all warnings are enabled.
			const bool EnableAllWarnings = false;
			if (!EnableAllWarnings)
			{
				D3D12_MESSAGE_ID denyIDs[] = {
					D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
					D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE
				};

				D3D12_INFO_QUEUE_FILTER filter = {};
				filter.DenyList.NumIDs = 2;
				filter.DenyList.pIDList = denyIDs;
				d3dInfoQueue->PushStorageFilter(&filter);
			}
		}
#endif

		CreateCommandQueues();
		CreateSwapchain(window);

		// Create synchronization objects and wait until assets have been uploaded to the GPU.
		{
			ThrowIfFailed(_d3dDevice->CreateFence(_fenceValues[_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_d3dFence)));
			_fenceValues[_frameIndex]++;

			// Create an event handle to use for frame synchronization.
			_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (_fenceEvent == nullptr)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}

			WaitIdle();
		}

		return Graphics::Initialize(window);
	}

	bool D3D12Graphics::WaitIdle()
	{
		// Schedule a Signal command in the queue.
		if (FAILED(_d3dDirectQueue->Signal(
			_d3dFence.Get(),
			_fenceValues[_frameIndex])))
		{
			ALIMER_LOGERROR("D3D12 failed to Signal fence");
			return false;
		}

		// Wait until the fence has been processed.
		if (FAILED(_d3dFence->SetEventOnCompletion(_fenceValues[_frameIndex], _fenceEvent)))
		{
			ALIMER_LOGERROR("D3D12 SetEventOnCompletion fail.");
			return false;
		}

		WaitForSingleObjectEx(_fenceEvent, INFINITE, FALSE);

		// Increment the fence value for the current frame.
		_fenceValues[_frameIndex]++;
		return true;
	}

	std::shared_ptr<Texture> D3D12Graphics::BeginFrame()
	{
		ThrowIfFailed(_commandAllocators[_frameIndex]->Reset());
		//ThrowIfFailed(_commandBuffers[_frameIndex]->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));

		return _textures[_frameIndex];
	}

	bool D3D12Graphics::Present()
	{
		// Present the frame.
		HRESULT hr = _swapChain->Present(1, 0);
		if (hr == DXGI_ERROR_DEVICE_REMOVED
			|| hr == DXGI_ERROR_DEVICE_RESET
			|| hr == DXGI_ERROR_DRIVER_INTERNAL_ERROR)
		{
			return false;
		}

		return MoveToNextFrame();
	}

	bool D3D12Graphics::MoveToNextFrame()
	{
		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = _fenceValues[_frameIndex];
		if (FAILED(_d3dDirectQueue->Signal(_d3dFence.Get(), currentFenceValue)))
		{
			return false;
		}

		// Update the frame index.
		_frameIndex = _swapChain->GetCurrentBackBufferIndex();

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (_d3dFence->GetCompletedValue() < _fenceValues[_frameIndex])
		{
			ThrowIfFailed(_d3dFence->SetEventOnCompletion(_fenceValues[_frameIndex], _fenceEvent));
			WaitForSingleObjectEx(_fenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		_fenceValues[_frameIndex] = currentFenceValue + 1;
		return true;
	}

	bool D3D12Graphics::InitializeCaps()
	{
		// https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/TechniqueDemos/D3D12MemoryManagement/src/Framework.cpp
		D3D12_FEATURE_DATA_D3D12_OPTIONS options;
		HRESULT hr = _d3dDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
		if (FAILED(hr))
		{
			ALIMER_LOGERROR("Failed to acquire D3D12 options for ID3D12Device 0x%p, hr=0x%.8x", _d3dDevice.Get(), hr);
			return false;
		}

		D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT gpuVaSupport;
		hr = _d3dDevice->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &gpuVaSupport, sizeof(gpuVaSupport));
		if (FAILED(hr))
		{
			ALIMER_LOGERROR("Failed to acquire GPU virtual address support for ID3D12Device 0x%p, hr=0x%.8x", _d3dDevice.Get(), hr);
			return false;
		}

		// Determine maximum supported feature level for this device
		static const D3D_FEATURE_LEVEL s_featureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels =
		{
			_countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0
		};

		hr = _d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

	void D3D12Graphics::CreateCommandQueues()
	{
		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.NodeMask = 0x0;

		if (FAILED(_d3dDevice->CreateCommandQueue(
			&queueDesc, IID_PPV_ARGS(&_d3dDirectQueue))))
		{
			ALIMER_LOGERROR("Failed to create D3D12 Direct CommandQueue");
			return;
		}

		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		if (FAILED(_d3dDevice->CreateCommandQueue(
			&queueDesc, IID_PPV_ARGS(&_d3dAsyncComputeQueue))))
		{
			ALIMER_LOGERROR("Failed to create D3D12 Async CommandQueue");
			return;
		}

		D3D12SetObjectName(_d3dDirectQueue.Get(), L"Direct Command Queue");
		D3D12SetObjectName(_d3dAsyncComputeQueue.Get(), L"Async Compute Command Queue");
	}

	void D3D12Graphics::CreateSwapchain(std::shared_ptr<Window> window)
	{
		// Describe and create the swap chain.
		// TODO: Add VSync support.
		// TODO: Add Tearing support.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = window->GetWidth();
		swapChainDesc.Height = window->GetHeight();
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;

#if !ALIMER_PLATFORM_UWP
		std::shared_ptr<WindowWindows> win32Window = std::static_pointer_cast<WindowWindows>(window);

		HWND handle = win32Window->GetHandle();
		ThrowIfFailed(_factory->CreateSwapChainForHwnd(
			_d3dDirectQueue.Get(),
			handle,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		// This sample does not support fullscreen transitions.
		ThrowIfFailed(_factory->MakeWindowAssociation(handle, DXGI_MWA_NO_ALT_ENTER));
#else
		ThrowIfFailed(_factory->CreateSwapChainForCoreWindow(
			_d3dDirectQueue.Get(),
			reinterpret_cast<IUnknown*>(Windows::UI::Core::CoreWindow::GetForCurrentThread()),
			&swapChainDesc,
			nullptr,
			&swapChain
		));
#endif

		ThrowIfFailed(swapChain.As(&_swapChain));
		_frameIndex = _swapChain->GetCurrentBackBufferIndex();

		_textures.resize(swapChainDesc.BufferCount);
		
		for (UINT i = 0; i < swapChainDesc.BufferCount; ++i)
		{
			// Get buffer from swapchain.
			ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&_renderTargets[i])));
			_textures[i] = std::make_shared<D3D12Texture>(this, _renderTargets[i].Get());

			ThrowIfFailed(_d3dDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&_commandAllocators[i])));
		}

		// Use single command list with different allocators.
		_commandBuffers.resize(swapChainDesc.BufferCount);
		_commandBuffers[0] = std::make_shared<D3D12CommandBuffer>(this);
	}
}
