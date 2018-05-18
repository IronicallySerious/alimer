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

#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#pragma warning(push)
#pragma warning(disable : 4467)
#include <wrl.h>
#pragma warning(pop)
using namespace Microsoft::WRL;

#include <vector>
#include "Graphics/PixelFormat.h"

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

namespace Alimer
{
	inline std::string HrToString(HRESULT hr)
	{
		char s_str[64] = {};
		sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
		return std::string(s_str);
	}

	class HrException : public std::runtime_error
	{
	public:
		HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
		HRESULT Error() const { return m_hr; }
	private:
		const HRESULT m_hr;
	};

	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw HrException(hr);
		}
	}

	inline void D3D12SetObjectName(ID3D12Object* object, _In_z_  LPCWSTR name)
	{
#if defined(ALIMER_DEV)
		object->SetName(name);
#endif
	}

	class D3D12Resource
	{
	public:
		ID3D12Resource * operator->() { return _resource.Get(); }
		const ID3D12Resource* operator->() const { return _resource.Get(); }

		ID3D12Resource* GetResource() { return _resource.Get(); }
		const ID3D12Resource* GetResource() const { return _resource.Get(); }

		D3D12_RESOURCE_STATES GetUsageState() const { return _usageState; }
		void SetUsageState(D3D12_RESOURCE_STATES state) { _usageState = state; }

		D3D12_RESOURCE_STATES GetTransitioningState() const { return _transitioningState; }
		void SetTransitioningState(D3D12_RESOURCE_STATES state) { _transitioningState = state; }

		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return _gpuVirtualAddress; }

	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> _resource;
		D3D12_RESOURCE_STATES _usageState = D3D12_RESOURCE_STATE_COMMON;
		D3D12_RESOURCE_STATES _transitioningState = (D3D12_RESOURCE_STATES)-1;
		D3D12_GPU_VIRTUAL_ADDRESS _gpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
	};

	static inline DXGI_FORMAT Convert(PixelFormat format)
	{
		switch (format)
		{
			case PixelFormat::R8UNorm:
				return DXGI_FORMAT_R8_UNORM;
			case PixelFormat::RG8UNorm:
				return DXGI_FORMAT_R8G8_UNORM;
			case PixelFormat::RGBA8UNorm:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case PixelFormat::BGRA8UNorm:
				return DXGI_FORMAT_B8G8R8A8_UNORM;

			case PixelFormat::Undefined:
			default:
				return DXGI_FORMAT_UNKNOWN;
		}
	}

	static inline D3D12_HEAP_PROPERTIES HeapProperties(
		D3D12_HEAP_TYPE type,
		UINT creationNodeMask = 1,
		UINT nodeMask = 1)
	{
		D3D12_HEAP_PROPERTIES heapProps;
		heapProps.Type = type;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = creationNodeMask;
		heapProps.VisibleNodeMask = nodeMask;
		return heapProps;
	}

	static inline D3D12_RESOURCE_DESC BufferResourceDesc(
		const D3D12_RESOURCE_ALLOCATION_INFO& resAllocInfo,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE)
	{
		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = resAllocInfo.Alignment;
		desc.Width = resAllocInfo.SizeInBytes;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = flags;
		return desc;
	}

	static inline D3D12_RESOURCE_DESC BufferResourceDesc(
		UINT64 width,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		UINT64 alignment = 0)
	{
		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = alignment;
		desc.Width = width;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = flags;
		return desc;
	}
}
