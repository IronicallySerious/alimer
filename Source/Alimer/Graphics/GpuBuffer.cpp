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
#include "../Debug/Log.h"

namespace Alimer
{
    GpuBuffer::GpuBuffer()
    {

    }

    GpuBuffer::~GpuBuffer()
    {
        Destroy();
    }

    void GpuBuffer::Destroy()
    {
        if (_handle != nullptr)
        {
            agpuDestroyBuffer(_handle);
            _handle = nullptr;
        }
    }

    bool GpuBuffer::Define(BufferUsage usage, uint64_t size, uint32_t stride, const void* initialData, const String& name)
    {
        Destroy();

        _usage = usage;
        _stride = stride;
        _size = size;

        AgpuBufferDescriptor descriptor = {};
        descriptor.usage = static_cast<AgpuBufferUsage>(usage);
        descriptor.size = size;
        descriptor.stride = stride;
        if (!name.IsEmpty())
        {
            descriptor.name = name.CString();
        }

        _handle = agpuCreateBuffer(&descriptor, initialData);
        return _handle != nullptr;
    }

    bool GpuBuffer::SetSubData(const void* pData)
    {
        return false;
    }

    bool GpuBuffer::SetSubData(uint32_t offset, uint32_t size, const void* pData)
    {
        if (offset + size > GetSize())
        {
            ALIMER_LOGERROR("Buffer subdata out of range");
            return false;
        }

        return false;
    }
}
