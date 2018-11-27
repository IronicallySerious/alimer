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

#include "D3D12Prerequisites.h"

namespace Alimer
{
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
}
