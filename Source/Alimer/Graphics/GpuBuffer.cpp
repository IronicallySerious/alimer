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
#include "../Graphics/GraphicsImpl.h"
#include "../Core/Log.h"

namespace Alimer
{
    GpuBuffer::GpuBuffer(Graphics* graphics, BufferUsageFlags usage, ResourceUsage resourceUsage)
        : GpuResource(graphics, GpuResourceType::Buffer, resourceUsage)
        , _usage(usage)
    {

    }

    GpuBuffer::GpuBuffer(Graphics* graphics, const GpuBufferDescription& description, const void* initialData)
        : GpuResource(graphics, GpuResourceType::Buffer)
    {
        _usage = description.usage;
        _stride = description.elementSize;
        _resourceUsage = description.resourceUsage;
        _size = _stride * description.elementCount;
        ALIMER_ASSERT(description.usage != BufferUsage::Unknown);
        ALIMER_ASSERT(_size != 0);

        Create(initialData);
    }

    GpuBuffer::~GpuBuffer()
    {
        Destroy();
    }

    void GpuBuffer::Destroy()
    {
        SafeDelete(_handle);
    }

    static const char* BufferUsageToString(BufferUsageFlags usage)
    {
        if (usage & BufferUsage::Vertex)
            return "vertex";
        if (usage & BufferUsage::Index)
            return "index";
        if (usage & BufferUsage::Uniform)
            return "uniform";

        return "unknown";
    }

    bool GpuBuffer::Create(const void* initialData)
    {
        if (!_graphics || !_graphics->IsInitialized())
            return false;

        _handle = _graphics->CreateBuffer(_usage, _size, _stride, _resourceUsage, initialData);
        if (!_handle)
        {
            ALIMER_LOGERROR("Failed to create {} buffer", BufferUsageToString(_usage));
            return false;
        }

        ALIMER_LOGDEBUG(
            "Created {} buffer [size: {}, stride {}]",
            BufferUsageToString(_usage),
            _size,
            _stride);

        return true;
    }

    bool GpuBuffer::SetData(uint32_t offset, uint32_t size, const void* data)
    {
        if (_handle && _resourceUsage == ResourceUsage::Immutable)
        {
            ALIMER_LOGERROR("Can not update immutable buffer");
            return false;
        }

        //if (_shadowData)
        //{
        //    memcpy(_shadowData.get() + offset, data, size);
        //}

        return _handle->SetData(offset, size, data);
    }
}
