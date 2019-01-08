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

#include "D3D11Cache.h"
#include "DeviceD3D11.h"
#include "../Shader.h"
#include "../D3D/D3DShaderCompiler.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace alimer
{
    D3D11Cache::D3D11Cache(DeviceD3D11* device)
        : _device(device)
    {
    }

    D3D11Cache::~D3D11Cache()
    {
        Clear();
    }

    void D3D11Cache::Clear()
    {
        _inputLayouts.clear();
    }

    ID3D11InputLayout* D3D11Cache::GetInputLayout(ShaderModule* shader, const RenderPipelineDescriptor* descriptor)
    {
        /*Hasher hasher;
        hasher.UInt64(shader->GetHash());
        hasher.Data(reinterpret_cast<const uint32_t*>(descriptor->vertexLayouts), sizeof(descriptor->vertexLayouts));

        auto hash = hasher.GetValue();
        auto it = _inputLayouts.find(hash);
        if (it != end(_inputLayouts))
        {
            return it->second.Get();
        }

        bool isEmpty = true;
        for (uint32_t i = 0; i < MaxVertexBufferBindings; i++)
        {
            if (descriptor->vertexLayouts[i].stride != 0)
            {
                isEmpty = false;
                break;
            }
        }

        std::vector<D3D11_INPUT_ELEMENT_DESC> d3dElementDescs;
        if (isEmpty)
        {
            // Generate vertex input format from shader reflection.
            std::vector<PipelineResource> inputs;

            // Only consider input resources to vertex shader stage.
            auto& reflection = shader->GetReflection();
            for (auto& resource : reflection.resources)
            {
                if (resource.resourceType == ResourceParamType::Input)
                {
                    inputs.push_back(resource);
                }
            }

            // Sort the vertex inputs by location value.
            std::sort(inputs.begin(), inputs.end(), [](const PipelineResource& a, const PipelineResource& b) {
                return (a.location < b.location);
            });

            // Create the VkVertexInputAttributeDescription objects.
            uint32_t offset = 0;
            for (auto& input : inputs)
            {
                // Assume all attributes are interleaved in the same buffer bindings.
                VertexElementSemantic semantic = static_cast<VertexElementSemantic>(input.location);
                D3D11_INPUT_ELEMENT_DESC inputElement = {};
                inputElement.SemanticName = d3d::Convert(semantic, inputElement.SemanticIndex);
                inputElement.Format = DXGI_FORMAT_UNKNOWN;
                inputElement.InputSlot = 0;
                inputElement.AlignedByteOffset = offset;
                inputElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                inputElement.InstanceDataStepRate = 0;

                uint32_t typeSize = 0;
                switch (input.dataType)
                {
                case ParamDataType::Char:
                    typeSize = sizeof(char);
                    if (input.vecSize == 1) inputElement.Format = DXGI_FORMAT_R8_SINT;
                    else if (input.vecSize == 2) inputElement.Format = DXGI_FORMAT_R8G8_SINT;
                    //else if (input.vecSize == 3) inputElement.Format = DXGI_FORMAT_R8G8B8_SINT;
                    else if (input.vecSize == 4) inputElement.Format = DXGI_FORMAT_R8G8B8A8_SINT;
                    break;

                case ParamDataType::Int:
                    typeSize = sizeof(int32_t);
                    if (input.vecSize == 1) inputElement.Format = DXGI_FORMAT_R32_SINT;
                    else if (input.vecSize == 2) inputElement.Format = DXGI_FORMAT_R32G32_SINT;
                    else if (input.vecSize == 3) inputElement.Format = DXGI_FORMAT_R32G32B32_SINT;
                    else if (input.vecSize == 4) inputElement.Format = DXGI_FORMAT_R32G32B32A32_SINT;
                    break;

                case ParamDataType::UInt:
                    typeSize = sizeof(uint32_t);
                    if (input.vecSize == 1) inputElement.Format = DXGI_FORMAT_R32_UINT;
                    else if (input.vecSize == 2) inputElement.Format = DXGI_FORMAT_R32G32_UINT;
                    else if (input.vecSize == 3) inputElement.Format = DXGI_FORMAT_R32G32B32_UINT;
                    else if (input.vecSize == 4) inputElement.Format = DXGI_FORMAT_R32G32B32A32_UINT;
                    break;

                case ParamDataType::Half:
                    typeSize = sizeof(uint16_t);
                    if (input.vecSize == 1) inputElement.Format = DXGI_FORMAT_R16_FLOAT;
                    else if (input.vecSize == 2) inputElement.Format = DXGI_FORMAT_R16G16_FLOAT;
                    //else if (input.vecSize == 3) inputElement.Format = DXGI_FORMAT_R16G16B16_FLOAT;
                    else if (input.vecSize == 4) inputElement.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                    break;

                case ParamDataType::Float:
                    typeSize = sizeof(float);
                    if (input.vecSize == 1) inputElement.Format = DXGI_FORMAT_R32_FLOAT;
                    else if (input.vecSize == 2) inputElement.Format = DXGI_FORMAT_R32G32_FLOAT;
                    else if (input.vecSize == 3) inputElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
                    else if (input.vecSize == 4) inputElement.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    break;

                default:
                    continue;
                }

                // Add the attribute description to the list.
                d3dElementDescs.push_back(inputElement);

                // Increase the stride for the next vertex attribute.
                offset += typeSize * input.vecSize;
            }
        }
        else
        {
        }*/

        ID3D11InputLayout* inputLayout = nullptr;
        /*auto byteCode = _vsBytecodes[shader->GetHash()];
        if (FAILED(_device->GetD3DDevice()->CreateInputLayout(
            d3dElementDescs.data(),
            static_cast<UINT>(d3dElementDescs.size()),
            byteCode.data(),
            byteCode.size(),
            &inputLayout)))
        {
            ALIMER_LOGERROR("D3D11 - Failed to create input layout");
        }

        _shaders[hash].Attach(inputLayout);*/
        return inputLayout;
    }
}
