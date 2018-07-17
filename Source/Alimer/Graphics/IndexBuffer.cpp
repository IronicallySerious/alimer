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

#include "../Graphics/IndexBuffer.h"
#include "../Graphics/GraphicsImpl.h"

namespace Alimer
{
    IndexBuffer::IndexBuffer(Graphics* graphics)
        : GpuBuffer(graphics, BufferUsage::Index)
    {
        _stride = 2;
    }

    IndexBuffer::~IndexBuffer()
    {
        Destroy();
    }

    bool IndexBuffer::Define(uint32_t indexCount, IndexType indexType, ResourceUsage resourceUsage, const void* data)
    {
        if (!indexCount)
        {
            ALIMER_LOGERROR("Can not define index buffer with no indices");
            return false;
        }

        if (resourceUsage == ResourceUsage::Immutable && !data)
        {
            ALIMER_LOGERROR("Immutable index buffer must define initial data");
            return false;
        }

        if (indexType != IndexType::UInt16
            && indexType != IndexType::UInt32)
        {
            ALIMER_LOGERROR("Index type must be UInt16 or UInt32");
            return false;
        }

        Destroy();
        _indexCount = indexCount;
        _stride = indexType == IndexType::UInt16 ? 2 : 4;
        _size = _indexCount * _stride;
        _resourceUsage = resourceUsage;

        const bool useShadowData = false;
        return GpuBuffer::Create(useShadowData, data);
    }

    bool IndexBuffer::SetData(const void* data, uint32_t indexStart, uint32_t indexCount)
    {
        //ALIMER_PROFILE(UpdateIndexBuffer);

        if (!data)
        {
            ALIMER_LOGERROR("Null source data for updating index buffer");
            return false;
        }

        if (indexStart == 0 && indexCount == 0)
        {
            // Entire buffer.
            return GpuBuffer::SetData(0, _size, data);
        }

        if (indexStart + indexCount > GetIndexCount())
        {
            ALIMER_LOGERROR("Out of bounds range for updating index buffer");
            return false;
        }

        if (indexCount == 0)
            indexCount = _indexCount - indexStart;

        return GpuBuffer::SetData(
            indexStart * _stride,
            indexCount * _stride,
            data);
    }
}
