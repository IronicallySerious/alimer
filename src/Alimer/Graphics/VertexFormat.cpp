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

#include "../Graphics/VertexFormat.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Core/Log.h"

namespace alimer
{
    VertexDeclaration::VertexDeclaration()
    {

    }

    void VertexDeclaration::Define(const Vector<VertexElement>& elements)
    {
        if (!elements.Size())
        {
            ALIMER_LOGERROR("Can not define vertex format with no elements");
        }

        Define(elements.Size(), elements.Data());
    }

    void VertexDeclaration::Define(const PODVector<VertexElement>& elements)
    {
        if (!elements.Size())
        {
            ALIMER_LOGERROR("Can not define vertex format with no elements");
        }

        Define(elements.Size(), elements.Data());
    }

    void VertexDeclaration::Define(uint32_t elementsCount, const VertexElement* elements)
    {
        if (!elementsCount || !elements)
        {
            ALIMER_LOGERROR("Can not define vertex format with no elements");
            return;
        }

        bool useAutoOffset = true;
        for (uint32_t i = 0; i < elementsCount; ++i)
        {
            if (elements[i].offset != 0)
            {
                useAutoOffset = false;
                break;
            }
        }

        _stride = 0;
        _elements.Resize(elementsCount);
        for (uint32_t i = 0; i < elementsCount; ++i)
        {
            _elements[i] = elements[i];
            _elements[i].offset = useAutoOffset ? _stride : elements[i].offset;
            _stride += GetVertexElementSize(elements[i].format);
            //elementHash |= ElementHash(i, elements[i].semantic);
        }
    }
}
