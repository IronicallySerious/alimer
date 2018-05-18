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

#include "D3D12DescriptorAllocator.h"
#include "D3D12Graphics.h"
#include "../../Debug/Log.h"

namespace Alimer
{
	D3D12DescriptorAllocator::D3D12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type)
		: _graphics(nullptr)
		, _type(type)
		, _currentHeap(nullptr)
		, _currentHandle{}
		, _descriptorSize(0)
		, _remainingFreeHandles(0)
	{
	}

	D3D12DescriptorAllocator::~D3D12DescriptorAllocator()
	{
	}

	void D3D12DescriptorAllocator::Initialize(D3D12Graphics* graphics)
	{
		_graphics = graphics;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorAllocator::Allocate(uint32_t count)
	{
		if (_currentHeap == nullptr
			|| _remainingFreeHandles < count)
		{
			_currentHeap = _graphics->RequestNewHeap(_type, NumDescriptorsPerHeap);
			_currentHandle = _currentHeap->GetCPUDescriptorHandleForHeapStart();
			_remainingFreeHandles = NumDescriptorsPerHeap;

			if (_descriptorSize == 0)
				_descriptorSize = _graphics->GetD3DDevice()->GetDescriptorHandleIncrementSize(_type);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE ret = _currentHandle;
		_currentHandle.ptr += count * _descriptorSize;
		_remainingFreeHandles -= count;
		return ret;
	}
}
