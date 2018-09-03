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

#include "D3D11GpuBuffer.h"
#include "D3D11Graphics.h"
#include "D3D11Convert.h"
#include "../../Math/MathUtil.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11GpuBuffer::D3D11GpuBuffer(D3D11Graphics* graphics, const BufferDescriptor* descriptor, const void* initialData)
        : GpuBuffer(graphics, descriptor)
    {
        if (descriptor->usage & BufferUsage::TransferSrc)
            _isDynamic = true;

        if (descriptor->usage & BufferUsage::TransferDest)
            _isDynamic = true;

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = static_cast<UINT>(descriptor->size);
        bufferDesc.StructureByteStride = descriptor->stride;
        bufferDesc.Usage = _isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        bufferDesc.CPUAccessFlags = 0;
        if (descriptor->usage & BufferUsage::TransferDest)
        {
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        }

        if (descriptor->usage & BufferUsage::TransferSrc)
        {
            bufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
        }

        if (descriptor->usage & BufferUsage::Uniform)
        {
            // D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT
            bufferDesc.ByteWidth = Align(bufferDesc.ByteWidth, 16u);
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        }
        else
        {
            if (descriptor->usage & BufferUsage::Vertex)
            {
                bufferDesc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
            }

            if (descriptor->usage & BufferUsage::Index)
            {
                bufferDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
            }

            if (descriptor->usage & BufferUsage::Storage)
            {
                bufferDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
            }

            if (descriptor->usage & BufferUsage::Indirect)
            {
                bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
            }
        }

        D3D11_SUBRESOURCE_DATA initData;
        memset(&initData, 0, sizeof(initData));
        initData.pSysMem = initialData;

        ThrowIfFailed(
            graphics->GetD3DDevice()->CreateBuffer(&bufferDesc, initialData ? &initData : nullptr, &_handle)
        );
    }

    D3D11GpuBuffer::~D3D11GpuBuffer()
    {
        if (_handle)
        {
#if defined(_DEBUG)
            ULONG refCount = GetRefCount(_handle);
            ALIMER_ASSERT_MSG(refCount == 1, "D3D11GpuBuffer leakage");
#endif

            _handle->Release();
            _handle = nullptr;
        }
    }

    bool D3D11GpuBuffer::SetSubDataImpl(GpuSize offset, GpuSize size, const void* pData)
    {
        ID3D11DeviceContext* d3dDeviceContext = StaticCast<D3D11Graphics>(_graphics)->GetD3DImmediateContext();

        if (_isDynamic)
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = d3dDeviceContext->Map(
                _handle,
                0,
                D3D11_MAP_WRITE_DISCARD,
                0,
                &mappedResource);

            if (FAILED(hr))
            {
                ALIMER_LOGERROR("Failed to map index buffer for update");
                return false;
            }

            memcpy(
                static_cast<uint8_t*>(mappedResource.pData) + offset,
                pData,
                size);

            d3dDeviceContext->Unmap(_handle, 0);
        }
        else
        {
            D3D11_BOX destBox;
            destBox.left = static_cast<UINT>(offset);
            destBox.right = static_cast<UINT>(size);
            destBox.top = destBox.front = 0;
            destBox.bottom = destBox.back = 1;

            d3dDeviceContext->UpdateSubresource(
                _handle,
                0,
                &destBox,
                pData,
                0,
                0);
        }

        return true;
    }
}
