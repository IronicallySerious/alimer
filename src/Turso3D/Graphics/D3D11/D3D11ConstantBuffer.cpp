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
#include "../../Math/Color.h"
#include "../../Math/Matrix3.h"
#include "../../Math/Matrix3x4.h"
#include "D3D11ConstantBuffer.h"
#include "D3D11Graphics.h"

#include <d3d11.h>

#include "../../Debug/DebugNew.h"

namespace Turso3D
{
    bool ConstantBuffer::SetData(const void* data, bool copyToShadow)
    {
        if (copyToShadow) {
            memcpy(_shadowData.Get(), data, _sizeInBytes);
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
                    memcpy((uint8_t*)mappedData.pData, data, _sizeInBytes);
                    d3dDeviceContext->Unmap(_handle, 0);
                }
                else
                {
                    TURSO3D_LOGERROR("Failed to map constant buffer for update");
                    return false;
                }
            }
            else
                d3dDeviceContext->UpdateSubresource(_handle, 0, nullptr, data, 0, 0);
        }

        dirty = false;
        return true;
    }
}
