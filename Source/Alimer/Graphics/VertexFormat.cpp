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

#include "../Graphics/VertexFormat.h"
#include "../Util/HashMap.h"

namespace Alimer
{
    VertexFormat::VertexFormat(const std::vector<VertexElement>& elements)
    {
        if (!elements.size())
        {
            ALIMER_LOGERROR("Can not define VertexElement with no elements");
            return;
        }

        Initialize(elements.data(), elements.size());
    }

    VertexFormat::VertexFormat(const VertexElement* elements, size_t elementCount)
    {
        if (!elementCount || !elements)
        {
            ALIMER_LOGERROR("Can not define VertexElement with no elements");
            return;
        }

        Initialize(elements, elementCount);
    }

    VertexFormat::~VertexFormat()
    {
    }

    void VertexFormat::Initialize(const VertexElement* elements, size_t elementCount)
    {
        // Check if we need to auto offset.
        bool useAutoOffset = true;
        for (size_t i = 0; i < elementCount; ++i)
        {
            if (elements[i].offset != 0)
            {
                useAutoOffset = false;
                break;
            }
        }

        uint32_t stride = 0;
        Hasher h;
        _elements.resize(elementCount);

        for (size_t i = 0; i < elementCount; ++i)
        {
            _elements[i] = elements[i];
            _elements[i].offset = useAutoOffset ? stride : elements[i].offset;
            stride += GetVertexFormatSize(elements[i].format);
            h.string(elements[i].semanticName);
            h.u32(elements[i].semanticIndex);
            h.u32(static_cast<uint32_t>(elements[i].format));
            h.u32(_elements[i].offset);
        }

        _stride = stride;
        _hash = h.get();
    }
}
