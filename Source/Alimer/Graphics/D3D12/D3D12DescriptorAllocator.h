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

#include "D3D12Helpers.h"
#include <mutex>
#include <vector>

namespace Alimer
{
	class D3D12Graphics;
	class D3D12DescriptorAllocator final
	{
	public:
		D3D12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type);
		~D3D12DescriptorAllocator();
		void Initialize(D3D12Graphics* graphics);

		D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t count);

	protected:
		static constexpr uint32_t NumDescriptorsPerHeap = 256;

		D3D12Graphics* _graphics;
		D3D12_DESCRIPTOR_HEAP_TYPE _type;
		ID3D12DescriptorHeap* _currentHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE _currentHandle;
		uint32_t _descriptorSize;
		uint32_t _remainingFreeHandles;
	};
}
