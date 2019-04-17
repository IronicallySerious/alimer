//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "../../Debug/Log.h"
#include "../../Debug/Profiler.h"
#include "D3D11Graphics.h"
#include "../Buffer.h"
#include "../VertexBuffer.h"
#include <d3d11.h>
#include "../../Debug/DebugNew.h"

namespace Turso3D
{
    void Buffer::Release()
    {
        if (_handle != nullptr)
        {
            if (bufferType == BufferType::Vertex)
            {
                for (uint32_t i = 0; i < MAX_VERTEX_STREAMS; ++i)
                {
                    if (graphics->GetVertexBuffer(i) == static_cast<VertexBuffer*>(this))
                        graphics->SetVertexBuffer(i, nullptr);
                }
            }

            _handle->Release();
            _handle = nullptr;
        }
    }

    /*bool VertexBuffer::SetData(size_t firstVertex, size_t numVertices_, const void* data)
    {
        TURSO3D_PROFILE(UpdateVertexBuffer);

        if (!data)
        {
            LOGERROR("Null source data for updating vertex buffer");
            return false;
        }
        if (firstVertex + numVertices_ > numVertices)
        {
            LOGERROR("Out of bounds range for updating vertex buffer");
            return false;
        }
        if (buffer && usage == ResourceUsage::Immutable)
        {
            LOGERROR("Can not update immutable vertex buffer");
            return false;
        }

        if (shadowData)
            memcpy(shadowData.Get() + firstVertex * vertexSize, data, numVertices_ * vertexSize);

        if (buffer)
        {
            ID3D11DeviceContext* d3dDeviceContext = (ID3D11DeviceContext*)graphics->D3DDeviceContext();

            if (usage == ResourceUsage::Dynamic)
            {
                D3D11_MAPPED_SUBRESOURCE mappedData;
                mappedData.pData = nullptr;

                d3dDeviceContext->Map((ID3D11Buffer*)buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
                if (mappedData.pData)
                {
                    memcpy((unsigned char*)mappedData.pData + firstVertex * vertexSize, data, numVertices_ * vertexSize);
                    d3dDeviceContext->Unmap((ID3D11Buffer*)buffer, 0);
                }
                else
                {
                    LOGERROR("Failed to map vertex buffer for update");
                    return false;
                }
            }
            else
            {
                D3D11_BOX destBox;
                destBox.left = (unsigned)(firstVertex * vertexSize);
                destBox.right = destBox.left + (unsigned)(numVertices_ * vertexSize);
                destBox.top = 0;
                destBox.bottom = 1;
                destBox.front = 0;
                destBox.back = 1;

                d3dDeviceContext->UpdateSubresource((ID3D11Buffer*)buffer, 0, &destBox, data, 0, 0);
            }
        }

        return true;
    }*/

    bool Buffer::Create(const void* data)
    {
        if (graphics
            && graphics->IsInitialized())
        {
            D3D11_BUFFER_DESC bufferDesc = {};
            D3D11_SUBRESOURCE_DATA initialData = {};
            initialData.pSysMem = data;

            switch (bufferType)
            {
            case BufferType::Vertex:
                bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                break;

            case BufferType::Index:
                bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                break;

            case BufferType::Uniform:
                bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                break;

            default:
                break;
            }

            bufferDesc.CPUAccessFlags = (usage == ResourceUsage::Dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
            bufferDesc.Usage = (D3D11_USAGE)usage;
            bufferDesc.ByteWidth = sizeInBytes;

            ID3D11Device* d3dDevice = (ID3D11Device*)graphics->D3DDevice();
            HRESULT hr = d3dDevice->CreateBuffer(&bufferDesc, data ? &initialData : nullptr, &_handle);

            if (FAILED(hr))
            {
                LOGERROR("Failed to create vertex buffer");
                return false;
            }

            //LOGDEBUGF("Created vertex buffer numVertices %u vertexSize %u", (unsigned)numVertices, (unsigned)vertexSize);
        }

        return true;
    }
}
