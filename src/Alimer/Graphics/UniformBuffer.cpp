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

#include "../Graphics/UniformBuffer.h"
#include "../Graphics/GPUDevice.h"
#include "../Core/Log.h"

namespace alimer
{
    void* alignedAlloc(size_t size, size_t alignment)
    {
        void *data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
        data = _aligned_malloc(size, alignment);
#else 
        int res = posix_memalign(&data, alignment, size);
        if (res != 0)
            data = nullptr;
#endif
        return data;
    }

    void alignedFree(void* data)
    {
#if	defined(_MSC_VER) || defined(__MINGW32__)
        _aligned_free(data);
#else 
        free(data);
#endif
    }

    UniformBuffer::UniformBuffer()
    {

    }

    bool UniformBuffer::Define(const PODVector<Uniform>& uniforms)
    {
        return Define(uniforms.Data(), uniforms.Size());
    }

    bool UniformBuffer::Define(const Uniform* uniforms, uint32_t count)
    {
        if (!count)
        {
            ALIMER_LOGERROR("Can not define empty uniform buffer");
            return false;
        }

        _uniforms.Clear();
        uint32_t size = 0;
        uint32_t minUboAlignment = _device->GetLimits().minUniformBufferOffsetAlignment;
        while (count--)
        {
            Uniform newConstant;
            newConstant.type = uniforms->type;
            newConstant.name = uniforms->name;
            newConstant.numElements = uniforms->numElements;
            newConstant.elementSize = GetUniformTypeSize(newConstant.type);
            // If element crosses 16 byte boundary or is larger than 16 bytes, align to next 16 bytes
            if ((newConstant.elementSize <= 16 && ((size + newConstant.elementSize - 1) >> 4) != (size >> 4)) ||
                (newConstant.elementSize > 16 && (size & 15)))
            {
                size += 16 - (size & 15);
            }

            newConstant.offset = size;
            _uniforms.Push(newConstant);
            size += newConstant.elementSize * newConstant.numElements;
            ++uniforms;
        }

        // Align the final buffer size to a multiple of 16 bytes
        uint32_t alignment = minUboAlignment;
        if (minUboAlignment > 0) {
            size = (size + minUboAlignment - 1) & ~(minUboAlignment - 1);
        }

        _shadowData = (uint8_t*)alignedAlloc(size, alignment);
        _descriptor.size = size;
        _descriptor.usage = BufferUsage::Uniform;
        _descriptor.stride = 1;
        return CreateGPUBuffer(false, nullptr);
    }

    uint32_t GetUniformTypeSize(UniformType type)
    {
        switch (type)
        {
        case UniformType::Float:
            return sizeof(float);

        case UniformType::Float2:
            return 2 * sizeof(float);

        case UniformType::Float3:
            return 3 * sizeof(float);
        case UniformType::Float4:
            return 4 * sizeof(float);

        case UniformType::Matrix3x3:
            return 3 * 3 * sizeof(float);

        case UniformType::Matrix3x4:
            return 3 * 4 * sizeof(float);

        case UniformType::Matrix4x4:
            return 4 * 4 * sizeof(float);

        default:
            ALIMER_UNREACHABLE();
        }
    }
}
