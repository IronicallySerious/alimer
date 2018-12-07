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
#include "../Graphics/Graphics.h"
#include "../Debug/Log.h"

namespace Alimer
{
    IndexBuffer::IndexBuffer()
        : GpuBuffer(Object::GetSubsystem<Graphics>())
    {
    }


    bool IndexBuffer::Define(ResourceUsage usage, uint32_t indexCount, IndexType indexType, const void* data)
    {
        if (!indexCount)
        {
            ALIMER_LOGERROR("Can not define index buffer with no indices");
            return false;
        }

        if (usage == ResourceUsage::Immutable && !data)
        {
            ALIMER_LOGERROR("Immutable index buffer must define initial data");
            return false;
        }

        if (indexType != IndexType::UInt16
            && indexType != IndexType::UInt32)
        {
            ALIMER_LOGERROR("Index buffer invalid index type");
            return false;
        }

        Destroy();

        // IndexBuffer attributes
        _indexCount = indexCount;
        _indexType = indexType;

        _usage = BufferUsage::Index;;
        _stride = indexType == IndexType::UInt32 ? 4 : 2;
        _size = _stride * indexCount;

        BufferDescriptor descriptor;
        descriptor.usage = _usage;
        descriptor.size = _size;
        descriptor.stride = _stride;
        return Create(&descriptor, data);
    }
}
