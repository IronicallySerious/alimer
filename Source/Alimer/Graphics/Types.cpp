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

#include "../Graphics/Types.h"
#include "../Graphics/Graphics.h"

namespace Alimer
{
    uint32_t GetVertexFormatSize(VertexElementFormat format)
    {
        switch (format)
        {
        case VertexElementFormat::Float:
        case VertexElementFormat::Byte4:
        case VertexElementFormat::Byte4N:
        case VertexElementFormat::UByte4:
        case VertexElementFormat::UByte4N:
        case VertexElementFormat::Short2:
        case VertexElementFormat::Short2N:
            return 4;
        case VertexElementFormat::Float2:
        case VertexElementFormat::Short4:
        case VertexElementFormat::Short4N:
            return 8;
        case VertexElementFormat::Float3:
            return 12;
        case VertexElementFormat::Float4:
            return 16;

        default:
            return static_cast<uint32_t>(-1);
        }
    }
}
