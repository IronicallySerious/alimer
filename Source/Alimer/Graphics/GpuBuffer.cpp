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

#include "../Graphics/GpuBuffer.h"
#include "../Graphics/Graphics.h"
#include "../Core/Log.h"

namespace Alimer
{
    GpuBuffer::GpuBuffer(Graphics* graphics, const BufferDescriptor* descriptor)
        : GpuResource(graphics, GpuResourceType::Buffer)
        , _usage(descriptor->usage)
        , _size(descriptor->size)
        , _stride(descriptor->stride)
    {
    }

    GpuBuffer::~GpuBuffer()
    {
        Destroy();
    }

    /*static const char* BufferUsageToString(BufferUsageFlags usage)
    {
        if (usage & BufferUsage::Vertex)
            return "vertex";
        if (usage & BufferUsage::Index)
            return "index";
        if (usage & BufferUsage::Uniform)
            return "uniform";

        return "unknown";
    }*/

    bool GpuBuffer::SetSubData(const void* pData)
    {
        if (!(_usage & BufferUsage::TransferDest))
        {
            ALIMER_LOGERROR("Buffer needs 'TransferDest' usage");
            return false;
        }

        return SetSubDataImpl(0, _size, pData);
    }

    bool GpuBuffer::SetSubData(uint32_t offset, uint32_t size, const void* pData)
    {
        if (offset + size > GetSize())
        {
            ALIMER_LOGERROR("Buffer subdata out of range");
            return false;
        }

        if (!(_usage & BufferUsage::TransferDest))
        {
            ALIMER_LOGERROR("Buffer needs 'TransferDest' usage");
            return false;
        }

        return SetSubDataImpl(offset, size, pData);
    }
}
