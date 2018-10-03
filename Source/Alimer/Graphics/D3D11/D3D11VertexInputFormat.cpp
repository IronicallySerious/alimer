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

#include "D3D11VertexInputFormat.h"
#include "D3D11GraphicsDevice.h"
#include "D3D11Convert.h"
#include "../D3D/D3DConvert.h"

namespace Alimer
{
    static int d3d11_SemanticIndices[static_cast<uint32_t>(VertexElementSemantic::Count)] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        1,
        2,
        3,
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7
    };

    D3D11VertexInputFormat::D3D11VertexInputFormat(D3D11Graphics* graphics, const VertexInputFormatDescriptor* descriptor)
        : VertexInputFormat(descriptor)
        , _graphics(graphics)
    {
        _inputElements.resize(_attributes.size());
        for (size_t i = 0, count = _attributes.size(); i < count; i++)
        {
            const VertexAttributeDescriptor& attribute = _attributes[i];

            _inputElements[i].SemanticName = d3d::Convert(attribute.semantic, _inputElements[i].SemanticIndex);
            _inputElements[i].Format = d3d::Convert(attribute.format);
            _inputElements[i].InputSlot = attribute.bufferIndex;
            _inputElements[i].AlignedByteOffset = attribute.offset;
            if (_layouts[attribute.bufferIndex].inputRate == VertexInputRate::Instance)
            {
                _inputElements[i].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
                _inputElements[i].InstanceDataStepRate = 1;
            }
            else
            {
                _inputElements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                _inputElements[i].InstanceDataStepRate = 0;
            }
        }
    }

    D3D11VertexInputFormat::~D3D11VertexInputFormat()
    {
    }
}
