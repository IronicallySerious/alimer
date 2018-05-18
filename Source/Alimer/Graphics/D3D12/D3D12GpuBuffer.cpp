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

#include "D3D12GpuBuffer.h"
#include "D3D12Graphics.h"
#include "../../Debug/Log.h"

namespace Alimer
{
	D3D12GpuBuffer::D3D12GpuBuffer(D3D12Graphics* graphics, BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData)
		: GpuBuffer(graphics, usage, elementCount, elementSize)
	{
		_usageState = D3D12_RESOURCE_STATE_COMMON;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = _size;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.Alignment = 0;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.MipLevels = 1;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;

		ThrowIfFailed(
			graphics->GetD3DDevice()->CreateCommittedResource(
				&heapProps,
				heapFlags,
				&resourceDesc,
				_usageState,
				nullptr,
				IID_PPV_ARGS(&_resource))
		);

		_gpuVirtualAddress = _resource->GetGPUVirtualAddress();
	}

	D3D12GpuBuffer::~D3D12GpuBuffer()
	{
	}
}
