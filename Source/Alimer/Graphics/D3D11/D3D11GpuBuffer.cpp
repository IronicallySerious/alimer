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
    
    D3D11GpuBuffer::D3D11GpuBuffer(D3D11Graphics* graphics, const GpuBufferDescription& description, const void* initialData)
        : GpuBuffer(graphics, description)
    {
        const bool dynamic = description.resourceUsage == ResourceUsage::Dynamic;

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = static_cast<UINT>(_size);
        bufferDesc.StructureByteStride = description.elementSize;
        bufferDesc.Usage = d3d11::Convert(description.resourceUsage);
        bufferDesc.CPUAccessFlags = 0;
        if (description.resourceUsage == ResourceUsage::Dynamic)
        {
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        }
        else if (description.resourceUsage == ResourceUsage::Staging)
        {
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
        }

        if (description.usage & BufferUsage::Uniform)
        {
            // D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT
            bufferDesc.ByteWidth = Align(bufferDesc.ByteWidth, 16u);
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        }
        else
        {
            if (description.usage & BufferUsage::Vertex)
            {
                bufferDesc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
            }

            if (description.usage & BufferUsage::Index)
            {
                bufferDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
            }

            if (description.usage & BufferUsage::Storage)
            {
                bufferDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
            }

            if (description.usage & BufferUsage::Indirect)
            {
                bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
            }
        }

        D3D11_SUBRESOURCE_DATA initData;
        memset(&initData, 0, sizeof(initData));
        initData.pSysMem = initialData;

        ThrowIfFailed(
            graphics->GetD3DDevice()->CreateBuffer(&bufferDesc, initialData ? &initData : nullptr, &_d3dBuffer)
        );
    }

    D3D11GpuBuffer::~D3D11GpuBuffer()
    {
        Destroy();
    }

    void D3D11GpuBuffer::Destroy()
    {
        if (_d3dBuffer)
        {
#if defined(_DEBUG)
            ULONG refCount = GetRefCount(_d3dBuffer);
            ALIMER_ASSERT_MSG(refCount == 1, "D3D11GpuBuffer leakage");
#endif

            _d3dBuffer->Release();
            _d3dBuffer = nullptr;
        }
    }
}
