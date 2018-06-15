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
#include "../../Core/Log.h"

namespace Alimer
{
    D3D12GpuBuffer::D3D12GpuBuffer(D3D12Graphics* graphics, const GpuBufferDescription& description, const void* initialData)
        : GpuBuffer(description)
    {
        // TODO: Property initialize using CommandList.
        _usageState = D3D12_RESOURCE_STATE_GENERIC_READ;

        CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(_size);

        HRESULT hr = graphics->GetD3DDevice()->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            _usageState,
            nullptr,
            IID_PPV_ARGS(&_resource)
        );

        if (initialData)
        {
            // Create staging buffer for copy data.
            //ComPtr<ID3D12Resource> stagingBuffer;

            //ThrowIfFailed(graphics->GetD3DDevice()->CreateCommittedResource(
            //	&HeapProperties(D3D12_HEAP_TYPE_UPLOAD),
            //	D3D12_HEAP_FLAG_NONE,
            //	&BufferResourceDesc(_size),
            //	D3D12_RESOURCE_STATE_GENERIC_READ,
            //	nullptr,
            //	IID_PPV_ARGS(&stagingBuffer)));

            UINT8* pDataBegin;
            D3D12_RANGE readRange = {};
            hr = _resource->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin));
            memcpy(pDataBegin, initialData, _size);
            _resource->Unmap(0, nullptr);
        }

        _gpuVirtualAddress = _resource->GetGPUVirtualAddress();
    }

    D3D12GpuBuffer::~D3D12GpuBuffer()
    {
    }
}
