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

#include "../Graphics/Buffer.h"
#include "../Graphics/Backend.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Core/Log.h"

namespace alimer
{
    Buffer::Buffer(BufferUsage usage)
        : GPUResource(Type::Buffer)
        , _usage(usage)
    {

    }

    Buffer::Buffer(GraphicsDevice* device, const BufferDescriptor* descriptor)
        : GPUResource(device, Type::Buffer)
        , _usage(descriptor->usage)
        , _size(descriptor->size)
        , _stride(descriptor->stride)
    {

    }

    Buffer::~Buffer()
    {
        Destroy();
    }

    void Buffer::Destroy()
    {
        SafeDelete(_handle);
    }

    bool Buffer::SetSubData(const void* pData)
    {
        return SetSubData(0, _size, pData);
    }

    bool Buffer::SetSubData(uint64_t offset, uint64_t size, const void* pData)
    {
        if (offset + size > GetSize())
        {
            ALIMER_LOGERROR("Buffer subdata out of range");
            return false;
        }

        return _handle->SetSubData(offset, size, pData);
    }

    bool Buffer::Create(bool useShadowData, const void* pData)
    {
        // If buffer is reinitialized with the same shadow data, no need to reallocate
        if (useShadowData && (!pData || pData != _shadowData.get()))
        {
            _shadowData.reset(new uint8_t[_size]);
            if (pData) {
                memcpy(_shadowData.get(), pData, _size);
            }
        }

        if (_device && _device->IsInitialized())
        {
            BufferDescriptor descriptor = {};
            descriptor.usage = _usage; /* TODO: Handle Storage and Indirect */
            descriptor.resourceUsage = _resourceUsage;
            descriptor.size = _size;
            descriptor.stride = _stride;
            _handle = _device->CreateBuffer(&descriptor, pData);
            return _handle != nullptr;
        }

        return true;
    }
}
