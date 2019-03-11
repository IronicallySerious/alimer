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
#include "GraphicsDeviceD3D11.h"
#include "D3D11Convert.h"
#include "Graphics/Shader.h"
#include "Graphics/D3D/D3DShaderCompiler.h"
#include "Graphics/D3D/D3DConvert.h"
#include "Core/Log.h"
using namespace Microsoft::WRL;

namespace alimer
{
    D3D11Cache::D3D11Cache(GraphicsDeviceD3D11* device)
        : _device(device)
    {
    }

    D3D11Cache::~D3D11Cache()
    {
        Clear();
    }

    void D3D11Cache::Clear()
    {
        _blendStates.clear();
        for (size_t i = 0; i < _samplerStates.size(); ++i)
        {
            SafeRelease(_samplerStates[i].state);
        }
        _samplerStates.clear();
        _inputLayouts.clear();
    }

    ID3D11BlendState* D3D11Cache::GetBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend)
    {
        Hasher hasher;
        hasher.UInt32(srcBlend);
        hasher.UInt32(destBlend);
        uint64_t hash = hasher.GetValue();
        auto it = _blendStates.find(hash);
        if (it != end(_blendStates))
        {
            return it->second.Get();
        }

        D3D11_BLEND_DESC desc = {};

        desc.RenderTarget[0].BlendEnable = (srcBlend != D3D11_BLEND_ONE) ||
            (destBlend != D3D11_BLEND_ZERO);

        desc.RenderTarget[0].SrcBlend = desc.RenderTarget[0].SrcBlendAlpha = srcBlend;
        desc.RenderTarget[0].DestBlend = desc.RenderTarget[0].DestBlendAlpha = destBlend;
        desc.RenderTarget[0].BlendOp = desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        ID3D11BlendState* blendState;
        HRESULT hr = _device->GetD3DDevice()->CreateBlendState(&desc, &blendState);
        if (FAILED(hr))
        {
            return nullptr;
        }

        _blendStates[hash].Attach(blendState);
        return blendState;
    }

    ID3D11SamplerState* D3D11Cache::GetSamplerState(const SamplerDescriptor* descriptor)
    {
        return nullptr;
        /*Hash stateHash = GenerateHash(descriptor, sizeof(SamplerDescriptor));

        const size_t numStates = _samplerStates.size();
        for (size_t i = 0; i < numStates; ++i)
        {
            if (_samplerStates[i].hash == stateHash)
            {
                return _samplerStates[i].state;
            }
        }

        D3D11_SAMPLER_DESC desc;
        memset(&desc, 0, sizeof desc);

        const bool isComparison = descriptor->compareFunction != CompareFunction::Never;
        const bool isAnisotropic = descriptor->maxAnisotropy > 1;
        desc.Filter = d3d11::Convert(descriptor->minFilter, descriptor->magFilter, descriptor->mipmapFilter, isComparison, isAnisotropic);
        desc.AddressU = d3d11::Convert(descriptor->addressModeU);
        desc.AddressV = d3d11::Convert(descriptor->addressModeV);
        desc.AddressW = d3d11::Convert(descriptor->addressModeV);
        desc.MipLODBias = 0.0f;
        desc.MaxAnisotropy = Max(descriptor->maxAnisotropy, 16u);
        desc.ComparisonFunc = d3d11::Convert(descriptor->compareFunction);
        desc.BorderColor[0] = 1.0f;
        desc.BorderColor[1] = 1.0f;
        desc.BorderColor[2] = 1.0f;
        desc.BorderColor[3] = 1.0f;
        desc.MinLOD = descriptor->lodMinClamp;
        desc.MaxLOD = descriptor->lodMaxClamp;

        ID3D11SamplerState* samplerState;
        HRESULT hr = _device->GetD3DDevice()->CreateSamplerState(&desc, &samplerState);
        if (FAILED(hr))
        {
            return nullptr;
        }

        CachedSamplerState cachedState;
        cachedState.hash = stateHash;
        cachedState.state = samplerState;
        _samplerStates.push_back(cachedState);
        return samplerState;*/
    }

    ID3D11InputLayout* D3D11Cache::GetInputLayout(ID3DBlob* blob, const RenderPipelineDescriptor* descriptor)
    {
        Hasher hasher;
        //hasher.Data(blob->GetBufferPointer(), blob->GetBufferSize());
        hasher.Data(reinterpret_cast<const uint32_t*>(&descriptor->vertexDescriptor), sizeof(descriptor->vertexDescriptor));

        uint64_t hash = hasher.GetValue();
        auto it = _inputLayouts.find(hash);
        if (it != end(_inputLayouts))
        {
            return it->second.Get();
        }

        bool isEmpty = true;
        for (uint32_t i = 0; i < MaxVertexBufferBindings; i++)
        {
            if (descriptor->vertexDescriptor.layouts[i].stride != 0)
            {
                isEmpty = false;
                break;
            }
        }

        D3D11_INPUT_ELEMENT_DESC d3d11InputElementDesc[MaxVertexAttributes];
        memset(d3d11InputElementDesc, 0, sizeof(d3d11InputElementDesc));
        if (isEmpty)
        {
            /*// Generate vertex input format from shader reflection.
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
            }*/
        }
        else
        {
            uint32_t index = 0;
            for (; index < MaxVertexAttributes; index++)
            {
                const VertexAttributeDescriptor* attributeDesc = &descriptor->vertexDescriptor.attributes[index];
                if (attributeDesc->format == VertexFormat::Unknown)
                {
                    break;
                }

                ALIMER_ASSERT((attributeDesc->bufferIndex >= 0) && (attributeDesc->bufferIndex < MaxVertexBufferBindings));
                const VertexBufferLayoutDescriptor* layoutDesc = &descriptor->vertexDescriptor.layouts[attributeDesc->bufferIndex];
                D3D11_INPUT_ELEMENT_DESC* inputElementDesc = &d3d11InputElementDesc[index];
                inputElementDesc->SemanticName = "TEXCOORD";
                inputElementDesc->SemanticIndex = index;
                inputElementDesc->Format = d3d::Convert(attributeDesc->format);
                inputElementDesc->InputSlot = attributeDesc->bufferIndex;
                inputElementDesc->AlignedByteOffset = attributeDesc->offset; // use_auto_offset ? auto_offset[a_desc->buffer_index] : a_desc->offset;
                if (layoutDesc->inputRate == VertexInputRate::Vertex)
                {
                    inputElementDesc->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                    inputElementDesc->InstanceDataStepRate = 0;
                    
                }
                else
                {
                    inputElementDesc->InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
                    inputElementDesc->InstanceDataStepRate = 1;
                }
                //auto_offset[a_desc->buffer_index] += _sg_vertexformat_bytesize(a_desc->format);
            }
        }

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
