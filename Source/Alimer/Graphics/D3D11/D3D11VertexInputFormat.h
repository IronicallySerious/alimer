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

#pragma once

#include "Graphics/VertexFormat.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
    class D3D11Graphics;

    /// D3D11 VertexInputFormat implementation.
    class D3D11VertexInputFormat final : public VertexInputFormat
    {
    public:
        /// Constructor.
        D3D11VertexInputFormat(D3D11Graphics* graphics, const VertexInputFormatDescriptor* descriptor);

        /// Destructor.
        ~D3D11VertexInputFormat() override;

        const std::vector<D3D11_INPUT_ELEMENT_DESC>& GetInputElements() const { return _inputElements; }

    private:
        D3D11Graphics* _graphics;
        std::vector<D3D11_INPUT_ELEMENT_DESC> _inputElements;
    };
}
