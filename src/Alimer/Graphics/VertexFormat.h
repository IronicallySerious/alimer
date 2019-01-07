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

#pragma once

#include "../Graphics/Types.h"

namespace alimer
{
	/// Defines a VertexFormat class.
	class ALIMER_API VertexFormat 
	{
	public:
        /// Constructor.
        VertexFormat(const PODVector<VertexElement>& elements);

        /// Constructor.
        VertexFormat(uint32_t elementsCount, const VertexElement* elements);

        /// Return stride of the format.
        uint32_t GetStride() const { return _stride; }

        /// Return number of vertex elements.
        uint32_t GetElementsCount() const { return _elements.Size(); }

        /// Return vertex elements.
        const PODVector<VertexElement>& GetElements() const { return _elements; }

    private:
        void Initialize(uint32_t elementsCount, const VertexElement* elements);

        PODVector<VertexElement> _elements;
        uint32_t _stride = 0;
	};
}