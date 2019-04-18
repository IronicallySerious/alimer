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

#include "../Debug/Log.h"
#include "../Debug/Profiler.h"
#include "Buffer.h"
#include "../Debug/DebugNew.h"

namespace Turso3D
{
    Buffer::Buffer(BufferType bufferType, ResourceUsage usage)
        : _bufferType(bufferType)
        , _usage(usage)
    {

    }

    Buffer::~Buffer()
    {
        Release();
    }

    bool Buffer::Create(bool useShadowData, const void* data)
    {
        // If buffer is reinitialized with the same shadow data, no need to reallocate
        if (useShadowData && (!data || data != _shadowData.Get()))
        {
            _shadowData = new uint8_t[_sizeInBytes];
            if (data) {
                memcpy(_shadowData.Get(), data, _sizeInBytes);
            }
        }

        // Destroy old handle first.
        Release();

        return CreateImpl(data);
    }
}
