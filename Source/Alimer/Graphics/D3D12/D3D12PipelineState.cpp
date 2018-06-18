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

#include "D3D12PipelineState.h"
#include "D3D12Shader.h"
#include "D3D12Graphics.h"
#include "../../Core/Log.h"


namespace Alimer
{
    static DXGI_FORMAT VertexFormatType(VertexFormat format)
    {
        switch (format)
        {
        case VertexFormat::Float:
            return DXGI_FORMAT_R32_FLOAT;
        case VertexFormat::Float2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case VertexFormat::Float3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case VertexFormat::Float4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case VertexFormat::Byte4:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case VertexFormat::Byte4N:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case VertexFormat::UByte4:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case VertexFormat::UByte4N:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case VertexFormat::Short2:
            return DXGI_FORMAT_R16G16_SINT;
        case VertexFormat::Short2N:
            return DXGI_FORMAT_R16G16_SNORM;
        case VertexFormat::Short4:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case VertexFormat::Short4N:
            return DXGI_FORMAT_R16G16B16A16_SNORM;
        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    D3D12PipelineState::D3D12PipelineState(D3D12Graphics* graphics, const RenderPipelineDescriptor& descriptor)
        : PipelineState(graphics, true)
    {
        _shader = StaticCast<D3D12Shader>(descriptor.shader);

        UINT elementsCount = 0;
        D3D12_INPUT_ELEMENT_DESC elements[MaxVertexAttributes];
        for (uint32_t i = 0; i < MaxVertexAttributes; ++i)
        {
            const VertexAttributeDescriptor& attribute = descriptor.vertexDescriptor.attributes[i];
            if (descriptor.vertexDescriptor.attributes[i].format == VertexFormat::Invalid)
                continue;

            // If the HLSL semantic is TEXCOORDN the SemanticName should be "TEXCOORD" and the
            // SemanticIndex N
            elements[i].SemanticName = "TEXCOORD";
            elements[i].SemanticIndex = static_cast<uint32_t>(i);
            elements[i].Format = VertexFormatType(attribute.format);
            elements[i].InputSlot = attribute.binding;
            elements[i].AlignedByteOffset = attribute.offset;
            elements[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            elements[i].InstanceDataStepRate = 0;
            elementsCount++;
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout.pInputElementDescs = elements;
        psoDesc.InputLayout.NumElements = elementsCount;
        psoDesc.pRootSignature = _shader->GetD3DRootSignature();

        std::vector<uint8_t> vsByteCode;
        std::vector<uint8_t> psByteCode;
        vsByteCode = _shader->AcquireBytecode(ShaderStage::Vertex);
        psByteCode = _shader->AcquireBytecode(ShaderStage::Fragment);

        psoDesc.VS = { vsByteCode.data(), vsByteCode.size() };
        psoDesc.PS = { psByteCode.data(), psByteCode.size() };

        // RasterizerState
        psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
        psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        psoDesc.RasterizerState.DepthClipEnable = TRUE;
        psoDesc.RasterizerState.MultisampleEnable = FALSE;
        psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
        psoDesc.RasterizerState.ForcedSampleCount = 0;
        psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        // BlendState
        psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
        psoDesc.BlendState.IndependentBlendEnable = FALSE;
        const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
        {
            FALSE,FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL,
        };
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;
        }

        // DepthStencilState
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.NodeMask = 1;

        if (FAILED(graphics->GetD3DDevice()->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&_pipelineState))))
        {
            ALIMER_LOGERROR("CreateGraphicsPipelineState failed");
            return;
        }

        _cbvDescriptorHandle = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
        _cbvHeap = _cbvDescriptorHandle.GetHeap();
    }

    D3D12PipelineState::~D3D12PipelineState() = default;

    void D3D12PipelineState::Bind(ID3D12GraphicsCommandList* commandList)
    {
        if (_isGraphics)
        {
            commandList->SetGraphicsRootSignature(_shader->GetD3DRootSignature());
        }
        else
        {
            commandList->SetComputeRootSignature(_shader->GetD3DRootSignature());
        }

        commandList->SetPipelineState(_pipelineState.Get());

        std::array<ID3D12DescriptorHeap *const, 2> heaps{
            _cbvHeap,
            _samplerHeap
        };

        commandList->SetDescriptorHeaps(_samplerHeap ? 2 : 1, heaps.data());
    }
}
