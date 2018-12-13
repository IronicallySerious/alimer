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
#include "D3D11GraphicsDevice.h"
#include "D3D11Convert.h"
#include "../../Math/MathUtil.h"
#include "../../Debug/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11Buffer::D3D11Buffer(D3D11Graphics* graphics)
        : GpuBuffer(graphics)
        , _device(graphics->GetD3DDevice())
        , _deviceContext(graphics->GetD3DDeviceContext())
    {
    }

    D3D11Buffer::~D3D11Buffer()
    {
        Destroy();
    }

    void D3D11Buffer::Destroy()
    {
        SafeRelease(_handle, "ID3D11Buffer");
    }

    bool D3D11Buffer::Create(const void* initialData)
    {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = static_cast<UINT>(_size);
        bufferDesc.StructureByteStride = _stride;
        bufferDesc.Usage = D3D11_USAGE_DEFAULT; // d3d11::Convert(descriptor->resourceUsage);
        bufferDesc.CPUAccessFlags = 0;

        /*if (descriptor->resourceUsage == ResourceUsage::Dynamic)
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (descriptor->resourceUsage == ResourceUsage::Staging)
        {
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        }*/

        if (any(_usage & BufferUsage::Uniform))
        {
            bufferDesc.ByteWidth = AlignTo(bufferDesc.ByteWidth, D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT);
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        }
        else
        {
            if (any(_usage & BufferUsage::Vertex))
            {
                bufferDesc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
            }

            if (any(_usage & BufferUsage::Index))
            {
                bufferDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
            }

            if (any(_usage & BufferUsage::Storage))
            {
                bufferDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
            }

            if (any(_usage & BufferUsage::Indirect))
            {
                bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
            }
        }

        D3D11_SUBRESOURCE_DATA initData;
        memset(&initData, 0, sizeof(initData));
        initData.pSysMem = initialData;
        HRESULT hr = _device->CreateBuffer(&bufferDesc, initialData ? &initData : nullptr, &_handle);
        return SUCCEEDED(hr);
    }

    /*bool D3D11Buffer::SetSubDataImpl(uint32_t offset, uint32_t size, const void* pData)
    {
        if (_resourceUsage == ResourceUsage::Dynamic)
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = _deviceContext->Map(
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

            _deviceContext->Unmap(_handle, 0);
        }
        else
        {
            D3D11_BOX destBox;
            destBox.left = offset;
            destBox.right = size;
            destBox.top = destBox.front = 0;
            destBox.bottom = destBox.back = 1;

            _deviceContext->UpdateSubresource(
                _handle,
                0,
                &destBox,
                pData,
                0,
                0);
        }

        return true;
    }*/
}
