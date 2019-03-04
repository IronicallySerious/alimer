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

#include "CommandContextD3D11.h"
#include "GraphicsDeviceD3D11.h"
#include "TextureD3D11.h"
#include "FramebufferD3D11.h"
#include "BufferD3D11.h"
#include "ShaderD3D11.h"
#include "PipelineD3D11.h"
#include "../D3D/D3DConvert.h"
#include "foundation/StringUtils.h"
#include "../../Math/MathUtil.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace alimer
{
    CommandContextD3D11::CommandContextD3D11(GraphicsDeviceD3D11* device)
        : CommandBuffer(device)
        , _immediate(true)
        , _context(device->GetD3DDeviceContext())
        , _fenceValue(0)
    {
        if (_context->GetType() == D3D11_DEVICE_CONTEXT_DEFERRED)
        {
            D3D11_FEATURE_DATA_THREADING threadingCaps = { FALSE, FALSE };

            HRESULT hr = device->GetD3DDevice()->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingCaps, sizeof(threadingCaps));
            if (SUCCEEDED(hr) && !threadingCaps.DriverCommandLists)
            {
                // The runtime emulates command lists.
                _needWorkaround = true;
            }
        }

        _context->QueryInterface(&_context1);
        _context->QueryInterface(&_annotation);
        BeginContext();
    }

    CommandContextD3D11::~CommandContextD3D11()
    {
        if (!_immediate)
        {
            SafeRelease(_context);
        }

        SafeRelease(_context1);
        SafeRelease(_annotation);
    }

    void CommandContextD3D11::BeginContext()
    {
        _colorRtvsCount = 0;
        _depthStencilView = nullptr;
        _graphicsPipeline = nullptr;
        _computePipeline = nullptr;
        _currentTopology = PrimitiveTopology::Count;

        // States
        _blendState = nullptr;
        _depthStencilState = nullptr;
        _rasterizerState = nullptr;

        // Shaders
        _currentVertexShader = nullptr;
        _currentPixelShader = nullptr;
        _currentComputeShader = nullptr;
        _currentInputLayout = nullptr;

        //_dirty = ~0u;
        _dirtySets = ~0u;
        _dirtyVbos = ~0u;
        //memset(&_bindings, 0, sizeof(_bindings));
        memset(_vbo.buffers, 0, sizeof(_vbo.buffers));

        _viewportsDirty = false; _viewportsCount = 1;
        _scissorsDirty = false; _scissorsCount = 1;

        _blendColor = Color4(1.0f, 1.0f, 1.0f, 1.0f);
        _stencilReference = 0;
    }

    uint64_t CommandContextD3D11::FlushImpl(bool waitForCompletion)
    {
        _context->Flush();
        return ++_fenceValue;
    }

    void CommandContextD3D11::PushDebugGroupImpl(const std::string& name, const Color4& color)
    {
        /* TODO: Use pix3 headers */
        std::wstring wideString = ToUtf16(name);
        _annotation->BeginEvent(wideString.c_str());
    }

    void CommandContextD3D11::PopDebugGroupImpl()
    {
        _annotation->EndEvent();
    }

    void CommandContextD3D11::InsertDebugMarkerImpl(const std::string& name, const Color4& color)
    {
        std::wstring wideString = ToUtf16(name);
        _annotation->SetMarker(wideString.c_str());
    }

    void CommandContextD3D11::BeginRenderPassImpl(const RenderPassDescriptor* renderPass)
    {
        uint32_t width = renderPass->renderTargetWidth == 0 ? UINT32_MAX : renderPass->renderTargetWidth;
        uint32_t height = renderPass->renderTargetHeight == 0 ? UINT32_MAX : renderPass->renderTargetHeight;
        uint32_t layers = renderPass->renderTargetArraySize == 0 ? UINT32_MAX : renderPass->renderTargetArraySize;

        for (uint32_t i = 0; i < renderPass->colorAttachmentCount; ++i)
        {
            auto texture = static_cast<TextureD3D11*>(renderPass->colorAttachments[i].texture);
            const uint32_t level = renderPass->colorAttachments[i].level;
            const uint32_t slice = renderPass->colorAttachments[i].slice;

            width = Min(width, texture->GetWidth(level));
            height = Min(height, texture->GetHeight(level));
            layers = Min(layers, texture->GetArraySize());
            _colorRtvs[_colorRtvsCount] = texture->GetRTV(level, slice);

            if (renderPass->colorAttachments[i].loadAction == LoadAction::Clear) {
                _context->ClearRenderTargetView(_colorRtvs[_colorRtvsCount], &renderPass->colorAttachments[i].clearColor[i]);
            }

            _colorRtvsCount++;
        }

        // Depth/stencil now.
        if (renderPass->depthStencilAttachment->texture != nullptr)
        {
            auto texture = static_cast<TextureD3D11*>(renderPass->depthStencilAttachment->texture);
            const uint32_t level = renderPass->depthStencilAttachment->level;
            const uint32_t slice = renderPass->depthStencilAttachment->slice;
            width = Min(width, texture->GetWidth(level));
            height = Min(height, texture->GetHeight(level));
            layers = Min(layers, texture->GetArraySize());

            _depthStencilView = texture->GetDSV(level, slice);

            UINT clearFlags = 0;
            if (renderPass->depthStencilAttachment->depthLoadAction == LoadAction::Clear)
            {
                clearFlags |= D3D11_CLEAR_DEPTH;
            }

            if ((renderPass->depthStencilAttachment->stencilLoadAction == LoadAction::Clear)
                && IsStencilFormat(texture->GetFormat()))
            {
                clearFlags |= D3D11_CLEAR_STENCIL;
            }

            if (clearFlags != 0)
            {
                _context->ClearDepthStencilView(
                    _depthStencilView,
                    clearFlags,
                    renderPass->depthStencilAttachment->clearDepth,
                    renderPass->depthStencilAttachment->clearStencil
                );
            }
        }

        _context->OMSetRenderTargets(_colorRtvsCount, _colorRtvs, _depthStencilView);

        // Set viewport and scissor from fbo.
        SetViewport(RectangleF(width, height));
        SetScissor(Rectangle(width, height));
    }

    void CommandContextD3D11::EndRenderPassImpl()
    {
        _context->OMSetRenderTargets(MaxColorAttachments, _nullRTVS, nullptr);
        _colorRtvsCount = 0;
        _depthStencilView = nullptr;
    }

    void CommandContextD3D11::SetViewport(const RectangleF& viewport)
    {
        _viewportsDirty = true;
        _viewportsCount = 1;
        _viewports[0] = { viewport.x, viewport.y, viewport.width, viewport.height, D3D11_MIN_DEPTH, D3D11_MAX_DEPTH };
    }

    void CommandContextD3D11::SetViewport(uint32_t viewportCount, const RectangleF* viewports)
    {
        _viewportsDirty = true;
        _viewportsCount = viewportCount;
        for (uint32_t i = 0; i < viewportCount; i++)
        {
            _viewports[i] = { viewports[i].x, viewports[i].y, viewports[i].width, viewports[i].height, D3D11_MIN_DEPTH, D3D11_MAX_DEPTH };
        }
    }

    void CommandContextD3D11::SetScissor(const Rectangle& scissor)
    {
        _scissorsDirty = true;
        _scissorsCount = 1;
        _scissors[0].left = static_cast<LONG>(scissor.x);
        _scissors[0].top = static_cast<LONG>(scissor.y);
        _scissors[0].right = static_cast<LONG>(scissor.x + scissor.width);
        _scissors[0].bottom = static_cast<LONG>(scissor.y + scissor.height);
    }

    void CommandContextD3D11::SetScissor(uint32_t scissorCount, const Rectangle* scissors)
    {
        _scissorsDirty = true;
        _scissorsCount = scissorCount;
        for (uint32_t i = 0; i < scissorCount; i++)
        {
            _scissors[i].left = static_cast<LONG>(scissors[i].x);
            _scissors[i].top = static_cast<LONG>(scissors[i].y);
            _scissors[i].right = static_cast<LONG>(scissors[i].x + scissors[i].width);
            _scissors[i].bottom = static_cast<LONG>(scissors[i].y + scissors[i].height);
        }
    }

    void CommandContextD3D11::SetBlendColor(const Color4& color)
    {
        _blendColor = color;
        //_context->OMSetBlendFactor()
        //_context->OMSetBlendState(blendState, blendConstants, 0xFFFFFFFF);
    }

    void CommandContextD3D11::SetStencilReference(uint32_t reference)
    {
        _stencilReference = reference;
    }

    void CommandContextD3D11::SetPipelineImpl(Pipeline* pipeline)
    {
        PipelineD3D11* pipelineD3D11 = static_cast<PipelineD3D11*>(pipeline);

        if (pipeline->IsCompute())
        {
            _computePipeline = pipelineD3D11;

            ID3D11ComputeShader* computeShader = _computePipeline->GetComputeShader();
            if (_currentComputeShader != computeShader)
            {
                _currentComputeShader = computeShader;
                _context->CSSetShader(computeShader, nullptr, 0);
            }
        }
        else
        {
            // Unset compute shader if any.
            if (_currentComputeShader != nullptr)
            {
                _context->CSSetShader(nullptr, nullptr, 0);
                _currentComputeShader = nullptr;
            }

            if (_graphicsPipeline != pipeline)
            {
                _graphicsPipeline = pipelineD3D11;

                // BlendState (TODO: Invalidate when calling SetBlendColor)
                ID3D11BlendState* blendState = pipelineD3D11->GetBlendState();
                if (_blendState != blendState)
                {
                    _blendState = blendState;
                    _context->OMSetBlendState(blendState, _blendColor.Data(), 0xFFFFFFFF);
                }

                // DepthStencilState (TODO: Invalidate when calling SetStencilReference)
                ID3D11DepthStencilState* depthStencilState = pipelineD3D11->GetDepthStencilState();
                //uint stencilReference = d3dPipeline.StencilReference;
                if (_depthStencilState != depthStencilState 
                    /*|| _stencilReference != stencilReference*/)
                {
                    _depthStencilState = depthStencilState;
                    //_stencilReference = stencilReference;
                    _context->OMSetDepthStencilState(depthStencilState, _stencilReference);
                }

                ID3D11RasterizerState* rasterizerState = pipelineD3D11->GetRasterizerState();
                if (_rasterizerState != rasterizerState)
                {
                    _rasterizerState = rasterizerState;
                    _context->RSSetState(rasterizerState);
                }


                ID3D11VertexShader* vertexShader = pipelineD3D11->GetVertexShader();
                if (_currentVertexShader != vertexShader)
                {
                    _currentVertexShader = vertexShader;
                    _context->VSSetShader(vertexShader, nullptr, 0);
                }

                ID3D11PixelShader* pixelShader = pipelineD3D11->GetPixelShader();
                if (_currentPixelShader != pixelShader)
                {
                    _currentPixelShader = pixelShader;
                    _context->PSSetShader(pixelShader, nullptr, 0);
                }
            }
        }
    }

    /*void CommandContextD3D11::SetVertexBufferImpl(uint32_t binding, Buffer* buffer, uint32_t offset, uint32_t stride, VertexInputRate inputRate)
    {
        auto d3dBuffer = static_cast<BufferD3D11*>(buffer)->GetHandle();
        if (_vbo.buffers[binding] != d3dBuffer
            || _vbo.offsets[binding] != offset)
        {
            _dirtyVbos |= 1u << binding;
        }

        if (_vbo.strides[binding] != stride
            || _vbo.inputRates[binding] != inputRate)
        {
            _vertexAttributesDirty = true;
        }

        _vbo.buffers[binding] = d3dBuffer;
        _vbo.offsets[binding] = offset;
        _vbo.strides[binding] = stride;
        _vbo.inputRates[binding] = inputRate;
    }

    void CommandContextD3D11::SetIndexBufferImpl(Buffer* buffer, uint32_t offset, IndexType indexType)
    {
        ID3D11Buffer* d3dBuffer = static_cast<BufferD3D11*>(buffer)->GetHandle();
        DXGI_FORMAT dxgiFormat = indexType == IndexType::UInt32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
        _context->IASetIndexBuffer(d3dBuffer, dxgiFormat, offset);
    }*/

    void CommandContextD3D11::DrawImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        FlushRenderState(topology);
        if (instanceCount <= 1)
        {
            _context->Draw(vertexCount, firstVertex);
        }
        else
        {
            _context->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
        }
    }

    void CommandContextD3D11::DrawIndexedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
    {
        FlushRenderState(topology);
        if (instanceCount <= 1)
        {
            _context->DrawIndexed(indexCount, firstIndex, baseVertex);
        }
        else
        {
            _context->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
        }
    }

    void CommandContextD3D11::DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        FlushComputeState();
        _context->Dispatch(groupCountX, groupCountY, groupCountZ);
    }

    /*void CommandContextD3D11::BindBufferImpl(GpuBuffer* buffer, uint32_t offset, uint32_t range, uint32_t set, uint32_t binding)
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

    void CommandContextD3D11::BindTextureImpl(Texture* texture, uint32_t set, uint32_t binding)
    {
        auto d3d11Texture = static_cast<TextureD3D11*>(texture);
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

    void CommandContextD3D11::FlushRenderState(PrimitiveTopology topology)
    {
        if (_currentTopology != topology)
        {
            _context->IASetPrimitiveTopology(d3d::Convert(topology, 1));
            _currentTopology = topology;
        }

        // Viewports
        if (_viewportsDirty)
        {
            _viewportsDirty = false;
            _context->RSSetViewports(_viewportsCount, _viewports);
        }

        // Scissors
        if (_scissorsDirty)
        {
            _scissorsDirty = false;
            _context->RSSetScissorRects(_scissorsCount, _scissors);
        }

        uint32_t activeVbos = 1u << 0u;
        uint32_t updateVboMask = _dirtyVbos & activeVbos;
        ForEachBitRange(updateVboMask, [&](uint32_t binding, uint32_t count)
            {
                for (uint32_t slot = binding; slot < binding + count; slot++)
                {
#ifdef ALIMER_DEV
                    ALIMER_ASSERT(_vbo.buffers[slot] != nullptr);
#endif
                    /*const PODVector<VertexElement>& elements = _vbo.formats[slot]->GetElements();

                    for (auto it = elements.Begin(); it != elements.End(); ++it)
                    {
                        const VertexElement& vertexElement = *it;
                        auto &inputElementDesc = _vertexAttributes[_vertexAttributesCount++];
                        inputElementDesc.SemanticName = d3d::Convert(vertexElement.semantic, inputElementDesc.SemanticIndex);
                        inputElementDesc.Format = d3d::Convert(vertexElement.format);
                        inputElementDesc.InputSlot = slot;
                        inputElementDesc.AlignedByteOffset = vertexElement.offset;

                        if (_vbo.inputRates[slot] == VertexInputRate::Instance)
                        {
                            inputElementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
                            inputElementDesc.InstanceDataStepRate = 1;
                        }
                        else
                        {
                            inputElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                            inputElementDesc.InstanceDataStepRate = 0;
                        }
                    }*/
                }

                _context->IASetVertexBuffers(binding, count, _vbo.buffers, _vbo.strides, _vbo.offsets);
            });

        _dirtyVbos &= ~updateVboMask;

        // InputLayout
        /*if (_vertexAttributesDirty)
        {
            _vertexAttributesDirty = false;

            const PODVector<uint8_t>& vsBlob = _graphicsShader->GetVertexShaderBlob();
            ID3D11InputLayout* inputLayout = nullptr;

            if (FAILED(_device->GetD3DDevice()->CreateInputLayout(
                _vertexAttributes,
                _vertexAttributesCount,
                vsBlob.Data(),
                vsBlob.Size(),
                &inputLayout)))
            {
                ALIMER_LOGERROR("D3D11 - Failed to create input layout");
            }
            else
            {
                //d3dGraphicsDevice->StoreInputLayout(newInputLayout, d3dInputLayout);
                _context->IASetInputLayout(inputLayout);
                //_currentInputLayout = newInputLayout;
            }
        }

        FlushDescriptorSets();*/
    }

    void CommandContextD3D11::FlushComputeState()
    {

    }

    /*void CommandContextD3D11::FlushDescriptorSet(uint32_t set)
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

    /*
    void D3D11CommandContext::DrawIndexedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
    {
        FlushRenderState(topology);
        _d3dContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void D3D11CommandContext::DrawIndexedInstancedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        FlushRenderState(topology);
        _d3dContext->DrawIndexedInstanced(indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }*/
}

