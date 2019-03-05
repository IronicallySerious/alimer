//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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
#include "../Graphics/Backend.h"
#include "../Graphics/GraphicsDevice.h"
#include "../foundation/Hash.h"
#include "../Core/Log.h"

namespace alimer
{
    IndexBuffer::IndexBuffer()
        : GPUResource(Type::Buffer)
    {

    }

    IndexBuffer::~IndexBuffer()
    {
        Destroy();
    }

    void IndexBuffer::Destroy()
    {
        SafeDelete(_handle);
    }

    bool IndexBuffer::Create(const void* data)
    {
        if (_device && _device->IsInitialized())
        {
            BufferDescriptor descriptor = {};
            descriptor.usage = BufferUsage::Index; /* TODO: Handle Storage and Indirect */
            descriptor.resourceUsage = _usage;
            descriptor.size = _size;
            descriptor.stride = _indexSize;
            descriptor.cpuAccessible = false;
            _handle = _device->CreateBuffer(&descriptor, data);
        }

        return true;
    }

    bool IndexBuffer::Define(uint32_t indexCount, IndexType indexType, bool useShadowData, ResourceUsage usage, const void* data)
    {
        if (!indexCount)
        {
            ALIMER_LOGERROR("Can not define index buffer with no indices");
            return false;
        }

        if (usage == ResourceUsage::Immutable
            && !data)
        {
            ALIMER_LOGERROR("Immutable index buffer must define initial data");
            return false;
        }

        Destroy();

        _indexCount = indexCount;
        _indexSize = indexType == IndexType::UInt16 ? 2 : 4;
        _size = _indexCount * _indexSize;
        _usage = usage;

        // If buffer is reinitialized with the same shadow data, no need to reallocate
        if (useShadowData && (!data || data != _shadowData.get()))
        {
            _shadowData.reset(new uint8_t[_size]);
            if (data) {
                memcpy(_shadowData.get(), data, _size);
            }
        }

        return Create(data);
    }
}