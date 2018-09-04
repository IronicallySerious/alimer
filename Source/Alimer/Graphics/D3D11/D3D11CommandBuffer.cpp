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

#include "D3D11CommandBuffer.h"
#include "D3D11Graphics.h"
#include "D3D11Texture.h"
#include "D3D11RenderPass.h"
#include "D3D11Shader.h"
#include "D3D11PipelineState.h"
#include "D3D11GpuBuffer.h"
#include "D3D11VertexInputFormat.h"
#include "../D3D/D3DConvert.h"
#include "../../Graphics/VertexFormat.h"
#include "../../Util/HashMap.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

static_assert(sizeof(D3D11_VIEWPORT) == sizeof(Alimer::Viewport), "D3D11_VIEWPORT mismatch");
static_assert(offsetof(Alimer::Viewport, x) == offsetof(D3D11_VIEWPORT, TopLeftX), "D3D11_VIEWPORT TopLeftX offset mismatch");
static_assert(offsetof(Alimer::Viewport, y) == offsetof(D3D11_VIEWPORT, TopLeftY), "D3D11_VIEWPORT TopLeftY offset mismatch");
static_assert(offsetof(Alimer::Viewport, width) == offsetof(D3D11_VIEWPORT, Width), "D3D11_VIEWPORT Width offset mismatch");
static_assert(offsetof(Alimer::Viewport, height) == offsetof(D3D11_VIEWPORT, Height), "D3D11_VIEWPORT Height offset mismatch");
static_assert(offsetof(Alimer::Viewport, minDepth) == offsetof(D3D11_VIEWPORT, MinDepth), "D3D11_VIEWPORT MinDepth offset mismatch");
static_assert(offsetof(Alimer::Viewport, maxDepth) == offsetof(D3D11_VIEWPORT, MaxDepth), "D3D11_VIEWPORT MaxDepth offset mismatch");

namespace Alimer
{
    static ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};

    // D3D11CommandContext
    D3D11CommandBuffer::D3D11CommandBuffer(D3D11Graphics* graphics, ID3D11DeviceContext1* context)
        : _graphics(graphics)
        , _context(context)
        , _immediate(context->GetType() == D3D11_DEVICE_CONTEXT_IMMEDIATE)
    {
        Reset();
    }

    D3D11CommandBuffer::~D3D11CommandBuffer()
    {
        if (!_immediate)
        {
            SafeRelease(_context);
        }
    }

    void D3D11CommandBuffer::Reset()
    {
        _currentRenderPass = nullptr;
        _currentColorAttachmentsBound = 0;
        _currentTopology = PrimitiveTopology::Count;
        _currentShader = nullptr;
        _currentRasterizerState = nullptr;
        _currentDepthStencilState = nullptr;
        _currentBlendState = nullptr;
        _currentVertexInputFormat = nullptr;

        _currentInputLayout.first = 0;
        _currentInputLayout.second = 0;

        //_dirtyVbos = ~0u;
        memset(_vbo.buffers, 0, sizeof(_vbo.buffers));
        memset(_vbo.d3dBuffers, 0, sizeof(_vbo.d3dBuffers));
        _inputLayoutDirty = false;
    }

    bool D3D11CommandBuffer::BeginCore()
    {
        if (!_immediate)
        {
            Reset();
        }

        return true;
    }

    bool D3D11CommandBuffer::EndCore()
    {
        return true;
    }

    void D3D11CommandBuffer::BeginRenderPassCore(RenderPass* renderPass, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil)
    {
        if (!renderPass)
        {
            renderPass = _graphics->GetBackbufferRenderPass();
        }

        _currentRenderPass = static_cast<D3D11RenderPass*>(renderPass);
        _currentRenderPass->Bind(_context);
        _currentColorAttachmentsBound = renderPass->GetColorAttachmentsCount();

        for (uint32_t i = 0; i < _currentColorAttachmentsBound; ++i)
        {
            const RenderPassAttachment& colorAttachment = renderPass->GetColorAttachment(i);

            switch (colorAttachment.loadAction)
            {
            case LoadAction::Clear:
                _context->ClearRenderTargetView(_currentRenderPass->GetRTV(i), &clearColors[i].r);
                break;

            default:
                break;
            }
        }

        // Set viewport and scissor from fbo.
        D3D11_VIEWPORT viewport = {
            0.0f, 0.0f,
            float(renderPass->GetWidth()), float(renderPass->GetHeight()),
            0.0f, 1.0f
        };

        D3D11_RECT scissor = { 0, 0,
            LONG(renderPass->GetWidth()), LONG(renderPass->GetHeight())
        };

        _context->RSSetViewports(1, &viewport);
        _context->RSSetScissorRects(1, &scissor);
    }

    void D3D11CommandBuffer::EndRenderPassCore()
    {
        _context->OMSetRenderTargets(_currentColorAttachmentsBound, nullViews, nullptr);
        _currentRenderPass = nullptr;
    }

    void D3D11CommandBuffer::SetViewport(const Viewport& viewport)
    {
        _context->RSSetViewports(1, (D3D11_VIEWPORT*)&viewport);
    }

    void D3D11CommandBuffer::SetScissor(const Rectangle& scissor)
    {
        D3D11_RECT scissorD3D;
        scissorD3D.left = scissor.x;
        scissorD3D.top = scissor.y;
        scissorD3D.right = scissor.x + scissor.width;
        scissorD3D.bottom = scissor.y + scissor.height;
        _context->RSSetScissorRects(1, &scissorD3D);
    }

    void D3D11CommandBuffer::SetShaderCore(Shader* shader)
    {
        if (_currentShader != shader)
        {
            _currentShader = static_cast<D3D11Shader*>(shader);
            _inputLayoutDirty = true;
        }
    }

    void D3D11CommandBuffer::BindVertexBufferImpl(uint32_t binding, GpuBuffer* buffer, GpuSize offset, uint64_t stride, VertexInputRate inputRate)
    {
        if (_vbo.buffers[binding] != buffer
            || _vbo.offsets[binding] != offset)
        {
            _vbo.d3dBuffers[binding] = static_cast<D3D11GpuBuffer*>(buffer)->GetHandle();

            _dirtyVbos |= 1u << binding;
        }

        if (_vbo.strides[binding] != stride
            || _vbo.inputRates[binding] != inputRate)
        {
            //set_dirty(COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT);
        }

        _vbo.buffers[binding] = buffer;
        _vbo.offsets[binding] = offset;
        _vbo.strides[binding] = stride;
        _vbo.inputRates[binding] = inputRate;
    }

    void D3D11CommandBuffer::SetVertexInputFormatImpl(VertexInputFormat* format)
    {
        if (_currentVertexInputFormat != format)
        {
            _currentVertexInputFormat = static_cast<D3D11VertexInputFormat*>(format);
            _inputLayoutDirty = true;
        }
    }

    void D3D11CommandBuffer::BindIndexBufferImpl(GpuBuffer* buffer, GpuSize offset, IndexType indexType)
    {
        ID3D11Buffer* d3dBuffer = static_cast<D3D11GpuBuffer*>(buffer)->GetHandle();
        DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R16_UINT;
        if (indexType == IndexType::UInt32)
        {
            dxgiFormat = DXGI_FORMAT_R32_UINT;
        }

        _context->IASetIndexBuffer(d3dBuffer, dxgiFormat, static_cast<UINT>(offset));
    }

    void D3D11CommandBuffer::BindBufferImpl(GpuBuffer* buffer, GpuSize offset, GpuSize range, uint32_t set, uint32_t binding)
    {
        ID3D11Buffer* d3dBuffer = static_cast<D3D11GpuBuffer*>(buffer)->GetHandle();
        _context->VSSetConstantBuffers(binding, 1, &d3dBuffer);
    }

    void D3D11CommandBuffer::SetTextureCore(uint32_t binding, Texture* texture, ShaderStageFlags stage)
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
    }

    bool D3D11CommandBuffer::PrepareDraw(PrimitiveTopology topology)
    {
        uint32_t updateVboMask = _dirtyVbos/* & _currentPipeline->GetBindingMask()*/;
        ForEachBitRange(updateVboMask, [&](uint32_t binding, uint32_t count)
        {
            UpdateVbos(binding, count);
        });
        _dirtyVbos &= ~updateVboMask;

        if (_currentTopology != topology)
        {
            _context->IASetPrimitiveTopology(d3d::Convert(topology));
            _currentTopology = topology;
        }

        _currentShader->Bind(_context);

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
        }*/

        return true;
    }

    void D3D11CommandBuffer::UpdateVbos(uint32_t binding, uint32_t count)
    {
#ifdef ALIMER_DEV
        for (uint32_t i = binding; i < binding + count; i++)
        {
            ALIMER_ASSERT(_vbo.buffers[i] != nullptr);
        }
#endif
        _context->IASetVertexBuffers(
            binding,
            count,
            _vbo.d3dBuffers + binding,
            _vbo.strides + binding,
            _vbo.offsets + binding);

        // InputLayout
        if (_inputLayoutDirty)
        {
            _inputLayoutDirty = false;

            InputLayoutDesc newInputLayout;
            newInputLayout.first = reinterpret_cast<uint64_t>(_currentVertexInputFormat);
            newInputLayout.second = _currentShader->GetVertexShaderHash();

            if (_currentInputLayout != newInputLayout)
            {
                // Check if layout already exists
                auto inputLayout = _graphics->GetInputLayout(newInputLayout);
                if (inputLayout != nullptr)
                {
                    _context->IASetInputLayout(inputLayout);
                    _currentInputLayout = newInputLayout;
                }
                else
                {
                    // Not found, create new.
                    std::vector<D3D11_INPUT_ELEMENT_DESC> d3dElementDescs;
                    if (_currentVertexInputFormat)
                    {
                        d3dElementDescs = _currentVertexInputFormat->GetInputElements();
                    }
                    else
                    {
                        // Generate vertex input format from shader reflection.
                        _currentShader->GetResourceLayout();
                    }

                    auto vsByteCode = _currentShader->AcquireVertexShaderBytecode();
                    ID3D11InputLayout* d3dInputLayout = nullptr;

                    if (FAILED(_graphics->GetD3DDevice()->CreateInputLayout(
                        d3dElementDescs.data(),
                        static_cast<UINT>(d3dElementDescs.size()),
                        vsByteCode.data(),
                        vsByteCode.size(), &d3dInputLayout)))
                    {
                        ALIMER_LOGERROR("D3D11 - Failed to create input layout");
                    }
                    else
                    {
                        _graphics->StoreInputLayout(newInputLayout, d3dInputLayout);
                        _context->IASetInputLayout(d3dInputLayout);
                        _currentInputLayout = newInputLayout;
                    }
                }
            }
        }
    }

    void D3D11CommandBuffer::DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
    {
        if (!PrepareDraw(topology))
            return;

        if (instanceCount <= 1)
        {
            _context->Draw(vertexCount, vertexStart);
        }
        else
        {
            _context->DrawInstanced(vertexCount, instanceCount, vertexStart, baseInstance);
        }
    }

    void D3D11CommandBuffer::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {
        if (!PrepareDraw(topology))
            return;

        if (instanceCount <= 1)
        {
            _context->DrawIndexed(indexCount, startIndex, 0);
        }
        else
        {
            _context->DrawIndexedInstanced(indexCount, instanceCount, startIndex, 0, 0);
        }
    }
}

