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

#include "DescriptorAllocatorD3D12.h"
#include "GraphicsDeviceD3D12.h"
#include "../../Core/Log.h"

namespace alimer
{
    D3D12DescriptorAllocator::D3D12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type)
        : _type(type)
        , _currentHeap(nullptr)
        , _currentCpuHandle{}
        , _currentGpuHandle{}
        , _descriptorSize(0)
        , _remainingFreeHandles(0)
    {
    }

    D3D12DescriptorAllocator::~D3D12DescriptorAllocator()
    {
    }

    D3D12DescriptorHandle D3D12DescriptorAllocator::Allocate(uint32_t count)
    {
        if (_currentHeap == nullptr
            || _remainingFreeHandles < count)
        {
            _currentHeap = _graphics->RequestNewHeap(_type, NumDescriptorsPerHeap);
            _currentCpuHandle = _currentHeap->GetCPUDescriptorHandleForHeapStart();
            if (_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                || _type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
            {
                _currentGpuHandle = _currentHeap->GetGPUDescriptorHandleForHeapStart();
            }

            _remainingFreeHandles = NumDescriptorsPerHeap;

            if (_descriptorSize == 0)
            {
                _descriptorSize = _graphics->GetD3DDevice()->GetDescriptorHandleIncrementSize(_type);
            }
        }

        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = _currentCpuHandle;
        _currentCpuHandle.ptr += count * _descriptorSize;
        _remainingFreeHandles -= count;

        if (_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
            || _type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = _currentGpuHandle;
            _currentGpuHandle.ptr += count * _descriptorSize;
            return D3D12DescriptorHandle(_currentHeap, _descriptorSize, cpuHandle, gpuHandle);
        }

        return D3D12DescriptorHandle(_currentHeap, _descriptorSize, cpuHandle);
    }
}
