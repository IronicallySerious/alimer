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
#include "D3D11VertexBuffer.h"

#include <d3d11.h>

#include "../../Debug/DebugNew.h"

namespace Turso3D
{
    
    bool VertexBuffer::SetData(const void* data, uint32_t vertexStart, uint32_t vertexCount)
    {
        TURSO3D_PROFILE(UpdateVertexBuffer);

        if (!data)
        {
            TURSO3D_LOGERROR("Null source data for updating vertex buffer");
            return false;
        }

        if (vertexCount == 0) {
            vertexCount = _vertexCount - vertexStart;
        }

        if (vertexStart + vertexCount > _vertexCount)
        {
            TURSO3D_LOGERROR("Out of bounds range for updating vertex buffer");
            return false;
        }

        if (_handle != nullptr && _usage == ResourceUsage::Immutable)
        {
            TURSO3D_LOGERROR("Can not update immutable vertex buffer");
            return false;
        }

        if (_shadowData)
        {
            memcpy(_shadowData.Get() + vertexStart * _vertexSize, data, vertexCount * _vertexSize);
        }

        if (_handle != nullptr)
        {
            ID3D11DeviceContext* d3dDeviceContext = (ID3D11DeviceContext*)graphics->D3DDeviceContext();

            if (_usage == ResourceUsage::Dynamic)
            {
                D3D11_MAPPED_SUBRESOURCE mappedData;
                mappedData.pData = nullptr;

                d3dDeviceContext->Map(_handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
                if (mappedData.pData)
                {
                    memcpy((uint8_t*)mappedData.pData + vertexStart * _vertexSize, data, vertexCount * _vertexSize);
                    d3dDeviceContext->Unmap(_handle, 0);
                }
                else
                {
                    TURSO3D_LOGERROR("Failed to map vertex buffer for update");
                    return false;
                }
            }
            else
            {
                D3D11_BOX destBox;
                destBox.left = vertexStart * _vertexSize;
                destBox.right = destBox.left + vertexCount * _vertexSize;
                destBox.top = 0;
                destBox.bottom = 1;
                destBox.front = 0;
                destBox.back = 1;

                d3dDeviceContext->UpdateSubresource(_handle, 0, &destBox, data, 0, 0);
            }
        }

        return true;
    }
}
