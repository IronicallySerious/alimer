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

#include "D3D11PipelineState.h"
#include "D3D11Shader.h"
#include "D3D11Graphics.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11PipelineState::D3D11PipelineState(D3D11Graphics* graphics, const RenderPipelineDescriptor& descriptor)
        : PipelineState(graphics, true)
    {
        _shader = StaticCast<D3D11Shader>(descriptor.shader);

        UINT elementsCount = 0;
        D3D11_INPUT_ELEMENT_DESC elements[MaxVertexAttributes];
        for (uint32_t i = 0; i < MaxVertexAttributes; ++i)
        {
            const VertexAttributeDescriptor& attribute = descriptor.vertexDescriptor.attributes[i];
            if (descriptor.vertexDescriptor.attributes[i].format == VertexFormat::Invalid)
                continue;

            // If the HLSL semantic is TEXCOORDN the SemanticName should be "TEXCOORD" and the
            // SemanticIndex N
            elements[i].SemanticName = "TEXCOORD";
            elements[i].SemanticIndex = static_cast<uint32_t>(i);
            elements[i].Format = d3d::Convert(attribute.format);
            elements[i].InputSlot = attribute.binding;
            elements[i].AlignedByteOffset = attribute.offset;
            elements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            elements[i].InstanceDataStepRate = 0;
            elementsCount++;
        }

        auto vsByteCode = _shader->AcquireVertexShaderBytecode();
        HRESULT hr = graphics->GetD3DDevice()->CreateInputLayout(
            elements, elementsCount,
            vsByteCode.data(), vsByteCode.size(),
            &_d3dInputLayout);

        if (FAILED(hr))
        {
            ALIMER_LOGERROR("D3D11 - Failed to create input layout");
        }
    }

    D3D11PipelineState::~D3D11PipelineState() = default;

    void D3D11PipelineState::Bind(ID3D11DeviceContext1* context)
    {
        if (_isGraphics)
        {
            context->IASetInputLayout(_d3dInputLayout);
        }
        else
        {
        }
    }
}
