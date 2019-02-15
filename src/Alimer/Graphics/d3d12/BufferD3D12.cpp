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

#include "BufferD3D12.h"
#include "GraphicsDeviceD3D12.h"
#include "../D3D/D3DConvert.h"
#include "../../Math/MathUtil.h"
#include "../../Core/Log.h"

namespace alimer
{
    BufferD3D12::BufferD3D12(GraphicsDeviceD3D12* device, const BufferDescriptor* descriptor, const void* initialData, void* externalHandle)
        : Buffer(device, descriptor)
    {
        _usageState = D3D12_RESOURCE_STATE_GENERIC_READ;

        const bool allowUAV = any(descriptor->usage & BufferUsage::Storage);

        const bool dynamic = any(descriptor->usage & BufferUsage::Dynamic);
        const bool cpuAccessible = any(descriptor->usage & BufferUsage::CPUAccessible);

        uint64_t size = AlignTo(descriptor->size, uint64_t(descriptor->stride));
        D3D12_RESOURCE_DESC resourceDesc = { };
        if (externalHandle == nullptr)
        {
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Width = dynamic ? size * RenderLatency : size;
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 1;
            resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            resourceDesc.Flags = allowUAV ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resourceDesc.Alignment = 0;

            const D3D12_HEAP_PROPERTIES* heapProps = cpuAccessible
                ? GetUploadHeapProps() : GetDefaultHeapProps();
            if (cpuAccessible)
            {
                _usageState = D3D12_RESOURCE_STATE_GENERIC_READ;
            }
            else if (initialData)
            {
                _usageState = D3D12_RESOURCE_STATE_COMMON;
            }

            ID3D12Heap* heap = nullptr;
            const uint64_t heapOffset = 0;
            if (heap)
            {
                ThrowIfFailed(device->GetD3DDevice()->CreatePlacedResource(
                    heap,
                    heapOffset,
                    &resourceDesc,
                    _usageState,
                    nullptr,
                    IID_PPV_ARGS(&_resource))
                );
            }
            else
            {
                ThrowIfFailed(device->GetD3DDevice()->CreateCommittedResource(
                    heapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    _usageState,
                    nullptr,
                    IID_PPV_ARGS(&_resource))
                );
            }
        }
        else 
        {
            _resource.Attach(static_cast<ID3D12Resource*>(externalHandle));
            resourceDesc = _resource->GetDesc();
        }

#if defined(ALIMER_DEV)
        //if (!descriptor->name.IsEmpty())
        //{
        //    _resource->SetName(WString(descriptor->name).CString());
        //}
#endif

        _gpuVirtualAddress = _resource->GetGPUVirtualAddress();

        if (cpuAccessible)
        {
            D3D12_RANGE readRange = { };
            ThrowIfFailed(_resource->Map(0, &readRange, reinterpret_cast<void**>(&_cpuAddress)));
        }

        if (initialData && cpuAccessible)
        {
            for (uint64_t i = 0; i < RenderLatency; ++i)
            {
                uint8_t* dstMem = _cpuAddress + size * i;
                memcpy(dstMem, initialData, size);
            }
        }
        else if (initialData)
        {
            UploadContext uploadContext = device->ResourceUploadBegin(resourceDesc.Width);

            memcpy(uploadContext.CPUAddress, initialData, size);
            if (dynamic)
                memcpy((uint8_t*)uploadContext.CPUAddress + size, initialData, size);

            uploadContext.commandList->CopyBufferRegion(
                _resource.Get(),
                0,
                uploadContext.resource,
                uploadContext.resourceOffset,
                size);

            device->ResourceUploadEnd(uploadContext);
        }
    }

    BufferD3D12::~BufferD3D12()
    {
        Destroy();
    }

    void BufferD3D12::Destroy()
    {
        if (_externalHandle) {
            return;
        }

#if !defined(NDEBUG)
        ULONG refCount = _resource.Reset();
        ALIMER_ASSERT_MSG(refCount == 0, "TextureD3D12 leakage");
#else
        _resource.Reset();
#endif
    }
}
