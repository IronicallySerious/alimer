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
#include "DeviceD3D11.h"
#include "TextureD3D11.h"
#include "FramebufferD3D11.h"
#include "BufferD3D11.h"
#include "ShaderD3D11.h"
//#include "D3D11Pipeline.h"
#include "../D3D/D3DConvert.h"
#include "../../Math/MathUtil.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace alimer
{
    CommandContextD3D11::CommandContextD3D11(DeviceD3D11* device)
        : _immediate(true)
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
        _renderTargetsViewsCount = 0;
        _graphicsShader = nullptr;
        _computeShader = nullptr;
        _currentTopology = PrimitiveTopology::Count;

        // States
        _currentRasterizerState = nullptr;
        _currentDepthStencilState = nullptr;
        _currentBlendState = nullptr;

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
        memset(_vbo.formats, 0, sizeof(_vbo.formats));

        _viewportsDirty = false; _viewportsCount = 1;
        _scissorsDirty = false; _scissorsCount = 1;
        _vertexAttributesDirty = false; _vertexAttributesCount = 0;
    }

    uint64_t CommandContextD3D11::Flush(bool waitForCompletion)
    {
        _context->Flush();
        return ++_fenceValue;
    }

    void CommandContextD3D11::PushDebugGroup(const char* name)
    {
        int bufferSize = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
        if (bufferSize > 0)
        {
            std::vector<WCHAR> buffer(bufferSize);
            MultiByteToWideChar(CP_UTF8, 0, name, -1, buffer.data(), bufferSize);
            _annotation->BeginEvent(buffer.data());
        }
    }

    void CommandContextD3D11::PopDebugGroup()
    {
        _annotation->EndEvent();
    }

    void CommandContextD3D11::InsertDebugMarker(const char* name)
    {
        int bufferSize = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
        if (bufferSize > 0)
        {
            std::vector<WCHAR> buffer(bufferSize);
            MultiByteToWideChar(CP_UTF8, 0, name, -1, buffer.data(), bufferSize);
            _annotation->SetMarker(buffer.data());
        }
    }

    void CommandContextD3D11::BeginRenderPass(const RenderPassDescriptor* descriptor)
    {
        _renderTargetsViewsCount = 0;

        for (uint32_t i = 0; i < MaxColorAttachments; ++i)
        {
            if (descriptor->colorAttachments[i].attachment.texture)
                continue;

            /*const AttachmentDescriptor& attachment = descriptor->colorAttachments[i].attachment;
            TextureD3D11* texture = static_cast<TextureD3D11*>(attachment.texture->GetGPUTexture());
            //_renderTargetsViews[_renderTargetsViewsCount] = 
            switch (descriptor->colorAttachments[i].loadAction)
            {
            case LoadAction::Clear:
                _context->ClearRenderTargetView(_currentFramebuffer->GetColorRTV(i), &descriptor->colorAttachments[i].clearColor[i]);
                break;

            default:
                break;
            }*/
            _renderTargetsViewsCount++;
        }

        // Depth/stencil now.
        if (descriptor->depthStencilAttachment.attachment.texture != nullptr)
        {
            const AttachmentDescriptor& attachment = descriptor->depthStencilAttachment.attachment;
            /*_depthStencilView = attachment->GetDSV();
            UINT clearFlags = 0;
            if (descriptor->depthStencilAttachment.depthLoadAction == LoadAction::Clear)
            {
                clearFlags |= D3D11_CLEAR_DEPTH;
            }

            if ((descriptor->depthStencilAttachment.stencilLoadAction == LoadAction::Clear)
                && IsStencilFormat(attachment.texture->GetFormat()))
            {
                clearFlags |= D3D11_CLEAR_STENCIL;
            }

            if (clearFlags != 0)
            {
                _context->ClearDepthStencilView(
                    _depthStencilView,
                    clearFlags,
                    descriptor->depthStencilAttachment.clearDepth,
                    descriptor->depthStencilAttachment.clearStencil
                );
            }*/
        }

        /*uint32_t width, height;
        if (!descriptor->renderTargetWidth)
        {
            width = framebuffer->GetWidth();
        }
        else
        {
            width = Min(framebuffer->GetWidth(), descriptor->renderTargetWidth);
        }

        if (!descriptor->renderTargetHeight)
        {
            height = framebuffer->GetHeight();
        }
        else
        {
            height = Min(framebuffer->GetHeight(), descriptor->renderTargetHeight);
        }

        // Set viewport and scissor from fbo.
        SetViewport(RectangleF(width, height));
        SetScissor(Rectangle(width, height));*/
    }

    void CommandContextD3D11::EndRenderPass()
    {
        static ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};

        _context->OMSetRenderTargets(_renderTargetsViewsCount, nullViews, nullptr);
        _renderTargetsViewsCount = 0;
    }

    /*void CommandContextD3D11::SetViewport(const RectangleF& viewport)
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

    void CommandContextD3D11::SetBlendColor(float r, float g, float b, float a)
    {
        //_context->OMSetBlendState(blendState, blendConstants, 0xFFFFFFFF);
    }

    void CommandContextD3D11::SetShaderImpl(Shader* shader)
    {
        ShaderD3D11* d3dShader = static_cast<ShaderD3D11*>(shader);
        if (any(d3dShader->GetStages() & ShaderStages::Compute))
        {
            _computeShader = d3dShader;

            ID3D11ComputeShader* computeShader = d3dShader->GetComputeShader();
            if (_currentComputeShader != computeShader)
            {
                _currentComputeShader = computeShader;
                _context->CSSetShader(computeShader, nullptr, 0);
            }
        }
        else
        {
            _graphicsShader = d3dShader;

            // Unset compute shader if any.
            if (_currentComputeShader != nullptr)
            {
                _context->CSSetShader(nullptr, nullptr, 0);
                _currentComputeShader = nullptr;
            }

            ID3D11VertexShader* vertexShader = d3dShader->GetVertexShader();
            if (_currentVertexShader != vertexShader)
            {
                _currentVertexShader = vertexShader;
                _context->VSSetShader(vertexShader, nullptr, 0);
            }

            ID3D11PixelShader* pixelShader = d3dShader->GetPixelShader();
            if (_currentPixelShader != pixelShader)
            {
                _currentPixelShader = pixelShader;
                _context->PSSetShader(pixelShader, nullptr, 0);
            }
        }
    }

    void CommandContextD3D11::SetVertexBufferImpl(uint32_t binding, Buffer* buffer, uint32_t offset, uint32_t stride, VertexInputRate inputRate)
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
    }

    void CommandContextD3D11::DrawInstancedImpl(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        FlushRenderState();
        if (instanceCount <= 1)
        {
            _context->Draw(vertexCount, firstVertex);
        }
        else
        {
            _context->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
        }
    }

    void CommandContextD3D11::DispatchImpl(uint32_t x, uint32_t y, uint32_t z)
    {
        _context->Dispatch(x, y, z);
    }*/

#if TODO_D3D11
    void CommandContextD3D11::SetPrimitiveTopologyCore(PrimitiveTopology topology)
    {
        if (_currentTopology != topology)
        {
            _d3dContext->IASetPrimitiveTopology(d3d::Convert(topology, 1));
            _currentTopology = topology;
        }
    }

#endif // TODO_D3D11

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

    void CommandContextD3D11::FlushRenderState()
    {
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
                    const PODVector<VertexElement>& elements = _vbo.formats[slot]->GetElements();

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
                    }
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
        }*/

        /*

        _currentD3DShader->Bind(_d3dContext);

        auto rasterizerState = _currentPipeline->GetD3DRasterizerState();
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

