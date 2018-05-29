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
#include "D3D12CommandListManager.h"
#include "D3D12Texture.h"
#include "D3D12CommandBuffer.h"
#include "D3D12GpuBuffer.h"
#include "D3D12PipelineLayout.h"
#include "D3D12Shader.h"
#include "D3D12PipelineState.h"
#include "../../Core/Log.h"
#include "../../IO/Path.h"
#include "../../IO/FileSystem.h"
#include "../../Core/Windows/EngineWindows.h"
#include "../../Core/Windows/WindowWindows.h"
#include "Shaders/Compiled/Triangle_VSMain.inc"
#include "Shaders/Compiled/Triangle_PSMain.inc"

#define SOURCE_SHADER_EXT ".hlsl"
#define COMPILED_SHADER_EXT ".cso"

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

	_Use_decl_annotations_ static void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
	{
		ComPtr<IDXGIAdapter1> adapter;
		*ppAdapter = nullptr;

		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
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

		*ppAdapter = adapter.Detach();
	}

	bool D3D12Graphics::IsSupported()
	{
		static bool availableCheck = false;
		static bool isAvailable = false;

		if (availableCheck)
			return isAvailable;

		availableCheck = true;
#if !ALIMER_PLATFORM_UWP
		if (FAILED(LoadD3D12Libraries()))
		{
			isAvailable = false;
			return false;
		}
#endif

		// Create temp dxgi factory for check support.
		ComPtr<IDXGIFactory2> factory;
		HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
		if (FAILED(hr))
		{
			isAvailable = false;
			return false;
		}

		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);
		isAvailable = hardwareAdapter != nullptr;
		return isAvailable;
	}

	D3D12Graphics::D3D12Graphics()
		: Graphics(GraphicsDeviceType::Direct3D12)
		, _commandListManager(nullptr)
		, _descriptorAllocator{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_TYPE_DSV }
	{

	}

	D3D12Graphics::~D3D12Graphics()
	{
		// Ensure that the GPU is no longer referencing resources that are about to be
		// cleaned up by the destructor.
		WaitIdle();

		// Delete command list manager.
		SafeDelete(_commandListManager);

		// Clear DescriptorHeap Pools.
		{
			std::lock_guard<std::mutex> guard(_heapAllocationMutex);
			_descriptorHeapPool.clear();
		}
	}

	bool D3D12Graphics::Initialize(std::shared_ptr<Window> window)
	{
#if !ALIMER_PLATFORM_UWP
		if (FAILED(LoadD3D12Libraries()))
		{
			ALIMER_LOGERROR("Failed to load D3D12 libraries.");
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
		// Create the command list manager class.
		_commandListManager = new D3D12CommandListManager(_d3dDevice.Get());

		// Init heaps.
		for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
		{
			_descriptorAllocator[i].Initialize(this);
		}

		// Create Swapchain.
		CreateSwapchain(window);

		// Init stock resources
		_stockVertexColor = std::make_shared<D3D12Shader>(this,
			ShaderBytecode{ sizeof(Triangle_VSMain), Triangle_VSMain },
			ShaderBytecode{ sizeof(Triangle_PSMain), Triangle_PSMain }
		);

		return Graphics::Initialize(window);
	}

	bool D3D12Graphics::WaitIdle()
	{
		return _commandListManager->WaitIdle();
	}

	std::shared_ptr<Texture> D3D12Graphics::AcquireNextImage()
	{
		uint32_t index = _swapChain->GetCurrentBackBufferIndex();
		return _textures[index];
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

	void D3D12Graphics::CreateSwapchain(std::shared_ptr<Window> window)
	{
		// Describe and create the swap chain.
		// TODO: Add VSync support.
		// TODO: Add Tearing support.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = window->GetWidth();
		swapChainDesc.Height = window->GetHeight();
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;

#if !ALIMER_PLATFORM_UWP
		std::shared_ptr<WindowWindows> win32Window = std::static_pointer_cast<WindowWindows>(window);

		HWND handle = win32Window->GetHandle();
		ThrowIfFailed(_factory->CreateSwapChainForHwnd(
			_commandListManager->GetD3DCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
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
			_commandListManager->GetD3DCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
			reinterpret_cast<IUnknown*>(Windows::UI::Core::CoreWindow::GetForCurrentThread()),
			&swapChainDesc,
			nullptr,
			&swapChain
		));
#endif

		ThrowIfFailed(swapChain.As(&_swapChain));

		_textures.resize(swapChainDesc.BufferCount);

		for (UINT i = 0; i < swapChainDesc.BufferCount; ++i)
		{
			// Get buffer from swapchain.
			ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&_renderTargets[i])));

			auto d3dTexture = std::make_shared<D3D12Texture>(this, _renderTargets[i].Get());
			d3dTexture->SetUsageState(D3D12_RESOURCE_STATE_PRESENT);
			_textures[i] = d3dTexture;
		}
	}

	CommandBufferPtr D3D12Graphics::CreateCommandBuffer()
	{
		auto d3dCommandBuffer = RetrieveCommandBuffer();
		if (d3dCommandBuffer == nullptr)
		{
			d3dCommandBuffer = std::make_shared<D3D12CommandBuffer>(this);
		}
		else
		{
			d3dCommandBuffer->Reset();
		}

		return d3dCommandBuffer;
	}

	GpuBufferPtr D3D12Graphics::CreateBuffer(BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData)
	{
		return std::make_shared<D3D12GpuBuffer>(this, usage, elementCount, elementSize, initialData);
	}

	PipelineLayoutPtr D3D12Graphics::CreatePipelineLayout()
	{
		return std::make_shared<D3D12PipelineLayout>(this);
	}

	std::shared_ptr<Shader> D3D12Graphics::CreateShader(const std::string& name)
	{
		// Lookup cache first.
		std::string shaderUrl = "shaders/cache/";
		shaderUrl.append(name);
		shaderUrl.append(COMPILED_SHADER_EXT);

		if (!FileSystem::Get().FileExists("assets://" + shaderUrl))
		{
			// Compile shader from source.
			shaderUrl = "shaders/";
			shaderUrl.append(name);
			shaderUrl.append(SOURCE_SHADER_EXT);
			if (!FileSystem::Get().FileExists("assets://" + shaderUrl))
			{
				ALIMER_LOGERROR("Source shader does not exists '%s'", shaderUrl.c_str());
				return nullptr;
			}

			std::string hlslSource = FileSystem::Get().ReadAllText("assets://" + shaderUrl);

			UINT compileFlags = 0;
#if defined(_DEBUG)
			// Enable better shader debugging with the graphics debugging tools.
			compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
			// Optimize.
			compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
			// SPRIV-cross does matrix multiplication expecting row major matrices
			compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

			const char* compileTarget = nullptr;
			std::string ext = Path::GetExtension(name);
			if (ext == "vert")
				compileTarget = "vs_5_0";
			else if (ext == "frag")
				compileTarget = "ps_5_0";

			ComPtr<ID3DBlob> shaderBlob;
			ComPtr<ID3DBlob> errors;
			if (FAILED(D3DCompile(
				hlslSource.c_str(),
				hlslSource.length(),
				nullptr,
				nullptr,
				nullptr,
				"main",
				compileTarget,
				compileFlags, 0,
				shaderBlob.ReleaseAndGetAddressOf(),
				errors.ReleaseAndGetAddressOf())))
			{
				ALIMER_LOGERROR("D3DCompile failed with error: %s", reinterpret_cast<char*>(errors->GetBufferPointer()));
				return nullptr;
			}

			return std::make_shared<D3D12Shader>(this, shaderBlob.Get());
		}

		// We could recompiled shader here, log error.
		if (!FileSystem::Get().FileExists("assets://" + shaderUrl))
		{
			ALIMER_LOGERROR("Compiled shader does not exists '%s'", shaderUrl.c_str());
			return nullptr;
		}

		return nullptr;
	}

	std::shared_ptr<Shader> D3D12Graphics::CreateShader(const ShaderBytecode& vertex, const ShaderBytecode& fragment)
	{
		return std::make_shared<D3D12Shader>(this, vertex, fragment);
	}

	PipelineStatePtr D3D12Graphics::CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor)
	{
		return std::make_shared<D3D12PipelineState>(this, descriptor);
	}

	std::shared_ptr<D3D12CommandBuffer> D3D12Graphics::RetrieveCommandBuffer()
	{
		std::lock_guard<std::mutex> lock(_commandBufferMutex);

		if (_commandBufferObjectId == 0)
			return nullptr;

		return _recycledCommandBuffers.at(--_commandBufferObjectId);
	}

	void D3D12Graphics::RecycleCommandBuffer(const std::shared_ptr<D3D12CommandBuffer>& cmd)
	{
		std::lock_guard<std::mutex> lock(_commandBufferMutex);

		if (_commandBufferObjectId < CommandBufferRecycleCount)
		{
			_recycledCommandBuffers.at(_commandBufferObjectId++) = cmd;
		}
	}

	ID3D12DescriptorHeap* D3D12Graphics::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
	{
		std::lock_guard<std::mutex> guard(_heapAllocationMutex);

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.Type = type;
		heapDesc.NumDescriptors = numDescriptors;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 1;

		ComPtr<ID3D12DescriptorHeap> heap;
		ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));
		_descriptorHeapPool.emplace_back(heap);
		return heap.Get();
	}
}
