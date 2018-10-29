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

#include "D3D11CommandContext.h"
#include "D3D11GraphicsDevice.h"
#include "D3D11Texture.h"
#include "D3D11Framebuffer.h"
#include "D3D11Shader.h"
#include "D3D11GpuBuffer.h"
#include "D3D11VertexInputFormat.h"
#include "../D3D/D3DConvert.h"
#include "../../Math/MathUtil.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11CommandContext::D3D11CommandContext(D3D11GraphicsDevice* device)
        : CommandContext(device)
        , _immediate(true)
        , _d3dContext(device->GetD3DDeviceContext())
        , _d3dContext1(device->GetD3DDeviceContext1())
    {
        CheckWorkaround();
        BeginContext();
    }

    D3D11CommandContext::~D3D11CommandContext()
    {
        if (!_immediate)
        {
            SafeRelease(_d3dContext);
            SafeRelease(_d3dContext1);
        }
    }

    void D3D11CommandContext::CheckWorkaround()
    {
        if (_d3dContext->GetType() == D3D11_DEVICE_CONTEXT_DEFERRED)
        {
            D3D11_FEATURE_DATA_THREADING threadingCaps = { FALSE, FALSE };

            HRESULT hr = static_cast<D3D11GraphicsDevice*>(_device)->GetD3DDevice()->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingCaps, sizeof(threadingCaps));
            if (SUCCEEDED(hr) && !threadingCaps.DriverCommandLists)
            {
                // The runtime emulates command lists.
                _needWorkaround = true;
            }
        }
    }

    void D3D11CommandContext::BeginContext()
    {
        _currentFramebuffer = nullptr;
        _currentColorAttachmentsBound = 0;
        _currentTopology = PrimitiveTopology::Count;
        _currentShader = nullptr;
        _currentRasterizerState = nullptr;
        _currentDepthStencilState = nullptr;
        _currentBlendState = nullptr;

        _currentInputLayout.first = 0;
        _currentInputLayout.second = 0;

        memset(&_bindings, 0, sizeof(_bindings));
        _inputLayoutDirty = false;
        memset(_currentVertexBuffers, 0, sizeof(_currentVertexBuffers));
        CommandContext::BeginContext();
    }

    void D3D11CommandContext::FlushImpl(bool waitForCompletion)
    {
        _d3dContext->Flush();
    }

    void D3D11CommandContext::BeginRenderPassImpl(Framebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor)
    {
        if (!framebuffer)
        {
            framebuffer = _device->GetSwapchainFramebuffer();
        }

        _currentFramebuffer = static_cast<D3D11Framebuffer*>(framebuffer);
        _currentColorAttachmentsBound = _currentFramebuffer->Bind(_d3dContext);

        for (uint32_t i = 0; i < _currentColorAttachmentsBound; ++i)
        {
            switch (descriptor[i].colors->loadAction)
            {
            case LoadAction::Clear:
                _d3dContext->ClearRenderTargetView(
                    _currentFramebuffer->GetColorRTV(i),
                    &descriptor[i].colors->clearColor[i]
                );
                break;

            default:
                break;
            }
        }

        // Depth/stencil now.
        auto depthStencilTexture = framebuffer->GetDepthStencilTexture();
        if (depthStencilTexture)
        {
            UINT clearFlags = 0;
            if (IsDepthFormat(depthStencilTexture->GetFormat())
                && (descriptor->depthStencil.depthLoadAction == LoadAction::Clear))
            {
                clearFlags |= D3D11_CLEAR_DEPTH;
            }

            if (IsStencilFormat(depthStencilTexture->GetFormat()) &&
                (descriptor->depthStencil.stencilLoadAction == LoadAction::Clear))
            {
                clearFlags |= D3D11_CLEAR_STENCIL;
            }

            if (clearFlags != 0)
            {
                _d3dContext->ClearDepthStencilView(
                    _currentFramebuffer->GetDSV(),
                    clearFlags,
                    descriptor->depthStencil.clearDepth,
                    descriptor->depthStencil.clearStencil);
            }
        }


        // Set viewport and scissor from fbo.
        D3D11_VIEWPORT viewport = {
            0.0f, 0.0f,
            static_cast<float>(_currentFramebuffer->GetWidth()),
            static_cast<float>(_currentFramebuffer->GetHeight()),
            0.0f, 1.0f
        };

        D3D11_RECT scissor = { 0, 0,
            static_cast<LONG>(_currentFramebuffer->GetWidth()),
            static_cast<LONG>(_currentFramebuffer->GetHeight())
        };

        _d3dContext->RSSetViewports(1, &viewport);
        _d3dContext->RSSetScissorRects(1, &scissor);
    }

    void D3D11CommandContext::EndRenderPassImpl()
    {
        static ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};

        _d3dContext->OMSetRenderTargets(_currentColorAttachmentsBound, nullViews, nullptr);
        _currentFramebuffer = nullptr;
    }

    void D3D11CommandContext::SetViewport(const rect& viewport)
    {
        D3D11_VIEWPORT d3dViewport = {
            viewport.x, viewport.y,
            viewport.width, viewport.height,
            D3D11_MIN_DEPTH, D3D11_MAX_DEPTH
        };
        _d3dContext->RSSetViewports(1, &d3dViewport);
    }

    void D3D11CommandContext::SetScissor(const irect& scissor)
    {
        D3D11_RECT scissorD3D;
        scissorD3D.left = scissor.x;
        scissorD3D.top = scissor.y;
        scissorD3D.right = scissor.x + scissor.width;
        scissorD3D.bottom = scissor.y + scissor.height;
        _d3dContext->RSSetScissorRects(1, &scissorD3D);
    }

    void D3D11CommandContext::SetShaderImpl(Shader* shader)
    {
        if (_currentD3DShader != shader)
        {
            _currentD3DShader = static_cast<D3D11Shader*>(shader);
            _inputLayoutDirty = true;
        }
    }

    void D3D11CommandContext::SetVertexBufferImpl(GpuBuffer* buffer, uint32_t offset)
    {
        auto d3dBuffer = static_cast<D3D11Buffer*>(buffer)->GetHandle();
        UINT stride = buffer->GetStride();
        _d3dContext->IASetVertexBuffers(0, 1, &d3dBuffer, &stride, &offset);
    }

    void D3D11CommandContext::SetVertexBuffersImpl(uint32_t firstBinding, uint32_t count, const GpuBuffer** buffers, const uint32_t* offsets)
    {
        for (uint32_t i = firstBinding; i < count; i++)
        {
            _currentVertexBuffers[i] = static_cast<const D3D11Buffer*>(buffers[i])->GetHandle();
            _vboStrides[i] = buffers[i]->GetStride();
            _vboOffsets[i] = offsets[i];
        }

        _d3dContext->IASetVertexBuffers(firstBinding, count, _currentVertexBuffers, _vboStrides, _vboOffsets);
    }

    void D3D11CommandContext::SetIndexBufferImpl(GpuBuffer* buffer, uint32_t offset, uint32_t stride)
    {
        ID3D11Buffer* d3dBuffer = static_cast<D3D11Buffer*>(buffer)->GetHandle();
        DXGI_FORMAT dxgiFormat = stride == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
        _d3dContext->IASetIndexBuffer(d3dBuffer, dxgiFormat, offset);
    }

    /*void D3D11CommandBuffer::BindBufferImpl(GpuBuffer* buffer, uint32_t offset, uint32_t range, uint32_t set, uint32_t binding)
    {
        auto &b = _bindings.bindings[set][binding];
        if (b.buffer.buffer == buffer
            && b.buffer.offset == offset
            && b.buffer.range == range)
        {
            return;
        }

        b.buffer = { buffer, offset,  Align(range, 16u) };
        _dirtySets |= 1u << set;
    }

    void D3D11CommandBuffer::BindTextureImpl(Texture* texture, uint32_t set, uint32_t binding)
    {
        auto d3d11Texture = static_cast<D3D11Texture*>(texture);
        ID3D11ShaderResourceView* shaderResourceView = d3d11Texture->GetShaderResourceView();
        ID3D11SamplerState* samplerState = d3d11Texture->GetSamplerState();

        if (stage & ShaderStage::Compute)
        {
            _context->CSSetShaderResources(binding, 1, &shaderResourceView);
            _context->CSSetSamplers(binding, 1, &samplerState);
        }

        if (stage & ShaderStage::Vertex)
        {
            _context->VSSetShaderResources(binding, 1, &shaderResourceView);
            _context->VSSetSamplers(binding, 1, &samplerState);
        }

        if (stage & ShaderStage::Fragment)
        {
            _context->PSSetShaderResources(binding, 1, &shaderResourceView);
            _context->PSSetSamplers(binding, 1, &samplerState);
        }
    }*/

    void D3D11CommandContext::FlushRenderState(PrimitiveTopology topology)
    {
        // InputLayout
        if (_inputLayoutDirty)
        {
            _inputLayoutDirty = false;

            InputLayoutDesc newInputLayout;
            newInputLayout.first = reinterpret_cast<uint64_t>(&_vertexDescriptor);
            newInputLayout.second = reinterpret_cast<uint64_t>(_currentD3DShader->GetVertexShaderBlob());

            if (_currentInputLayout != newInputLayout)
            {
                // Check if layout already exists
                auto d3dGraphicsDevice = static_cast<D3D11GraphicsDevice*>(_device);
                auto inputLayout = d3dGraphicsDevice->GetInputLayout(newInputLayout);
                if (inputLayout != nullptr)
                {
                    _d3dContext->IASetInputLayout(inputLayout);
                    _currentInputLayout = newInputLayout;
                }
                else
                {
                    // Not found, create new.
                    std::vector<D3D11_INPUT_ELEMENT_DESC> d3dElementDescs;
                    //if (_currentVertexInputFormat)
                    {
                        // Check if we need to auto offset.
                        bool useAutoOffset = true;
                        for (uint32_t i = 0; i < MaxVertexAttributes; i++)
                        {
                            if (_vertexDescriptor.attributes[i].format == VertexFormat::Invalid)
                                continue;

                            if (_vertexDescriptor.attributes[i].offset != 0)
                            {
                                useAutoOffset = false;
                                break;
                            }
                        }

                        uint32_t vertexStride = 0;
                        for (uint32_t i = 0; i < MaxVertexAttributes; i++)
                        {
                            const auto& attribute = _vertexDescriptor.attributes[i];
                            if (attribute.format == VertexFormat::Invalid)
                                continue;

                            D3D11_INPUT_ELEMENT_DESC d3dElement = {};
                            d3dElement.SemanticName = d3d::Convert(attribute.semantic, d3dElement.SemanticIndex);
                            d3dElement.Format = d3d::Convert(attribute.format);
                            d3dElement.InputSlot = attribute.bufferIndex;
                            if (_vertexDescriptor.buffers[attribute.bufferIndex].inputRate == VertexInputRate::Instance)
                            {
                                d3dElement.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
                                d3dElement.InstanceDataStepRate = 1;
                            }
                            else
                            {
                                d3dElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                                d3dElement.InstanceDataStepRate = 0;
                            }

                            d3dElement.AlignedByteOffset = useAutoOffset ? vertexStride : attribute.offset;
                            vertexStride += GetVertexFormatSize(attribute.format);
                            d3dElementDescs.push_back(d3dElement);
                        }
                    }
                    /*else
                    {
                        // Generate vertex input format from shader reflection.
                        std::vector<PipelineResource> inputs;

                        // Only consider input resources to vertex shader stage.
                        auto& resources = _currentShader->GetShader(ShaderStage::Vertex)->GetResources();
                        for (auto& resource : resources)
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
                        uint32_t stride = 0;
                        for (auto& input : inputs)
                        {
                            // Assume all attributes are interleaved in the same buffer bindings.
                            D3D11_INPUT_ELEMENT_DESC inputElement = {};
                            inputElement.SemanticName = "TEXCOORD";
                            inputElement.SemanticIndex = input.location;
                            inputElement.Format;
                            inputElement.InputSlot = 0;
                            inputElement.AlignedByteOffset = stride;
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
                            stride += typeSize * input.vecSize;
                        }
                    }*/

                    ID3DBlob* vsBlob = _currentD3DShader->GetVertexShaderBlob();
                    ID3D11InputLayout* d3dInputLayout = nullptr;

                    if (FAILED(d3dGraphicsDevice->GetD3DDevice()->CreateInputLayout(
                        d3dElementDescs.data(),
                        static_cast<UINT>(d3dElementDescs.size()),
                        vsBlob->GetBufferPointer(),
                        vsBlob->GetBufferSize(),
                        &d3dInputLayout)))
                    {
                        ALIMER_LOGERROR("D3D11 - Failed to create input layout");
                    }
                    else
                    {
                        d3dGraphicsDevice->StoreInputLayout(newInputLayout, d3dInputLayout);
                        _d3dContext->IASetInputLayout(d3dInputLayout);
                        _currentInputLayout = newInputLayout;
                    }
                }
            }
        }

        if (_currentTopology != topology)
        {
            _d3dContext->IASetPrimitiveTopology(d3d::Convert(topology));
            _currentTopology = topology;
        }

        _currentD3DShader->Bind(_d3dContext);

        /*auto rasterizerState = _currentPipeline->GetD3DRasterizerState();
        if (_currentRasterizerState != rasterizerState)
        {
            _currentRasterizerState = rasterizerState;
            _context->RSSetState(rasterizerState);
        }

        auto dsState = _currentPipeline->GetD3DDepthStencilState();
        if (_currentDepthStencilState != dsState)
        {
            _currentDepthStencilState = dsState;
            _context->OMSetDepthStencilState(dsState, 0);
        }

        auto blendState = _currentPipeline->GetD3DBlendState();
        if (_currentBlendState != blendState)
        {
            _currentBlendState = blendState;
            _context->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
        }

        FlushDescriptorSets();*/
    }

    /*void D3D11CommandBuffer::FlushDescriptorSet(uint32_t set)
    {
        // TODO: We need to remap from vulkan to d3d11 binding scheme.
        if (_needWorkaround)
        {
            // Workaround for command list emulation
            ID3D11Buffer* NullCBuf = nullptr;
            _context->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, &NullCBuf);
        }

        ID3D11Buffer* buffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {};
        UINT offsets[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {};
        UINT ranges[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {};

        for (uint32_t binding = 0; binding < MaxBindingsPerSet; ++binding)
        {
            auto &b = _bindings.bindings[set][binding];

            if (b.buffer.buffer == nullptr)
                continue;

            buffers[binding] = static_cast<D3D11Buffer*>(b.buffer.buffer)->GetHandle();
            offsets[binding] = b.buffer.offset;
            ranges[binding] = b.buffer.range;
        }

        _context->VSSetConstantBuffers1(
            0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT,
            buffers, offsets, ranges);
    }

    void D3D11CommandBuffer::FlushDescriptorSets()
    {
        uint32_t updateSet = 1; // layout.descriptorSetMask & _dirtySets;
        ForEachBit(updateSet, [&](uint32_t set) { FlushDescriptorSet(set); });
        _dirtySets &= ~updateSet;
    }*/

    void D3D11CommandContext::DrawImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t startVertexLocation)
    {
        FlushRenderState(topology);
        _d3dContext->Draw(vertexCount, startVertexLocation);
    }

    void D3D11CommandContext::DrawInstancedImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        FlushRenderState(topology);
        _d3dContext->DrawInstanced(vertexCount, instanceCount, startVertexLocation, startInstanceLocation);
    }

    void D3D11CommandContext::DrawIndexedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
    {
        FlushRenderState(topology);
        _d3dContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void D3D11CommandContext::DrawIndexedInstancedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        FlushRenderState(topology);
        _d3dContext->DrawIndexedInstanced(indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void D3D11CommandContext::SetPipelineImpl(Pipeline* pipeline)
    {

    }

    void D3D11CommandContext::DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        _d3dContext->Dispatch(groupCountX, groupCountY, groupCountZ);
    }
}

