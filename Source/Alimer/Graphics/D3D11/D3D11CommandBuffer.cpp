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
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11CommandBuffer::D3D11CommandBuffer(D3D11Graphics* graphics, ID3D11DeviceContext1* context)
        : CommandBuffer(graphics)
        , _context(context)
        , _isImmediate(true)
    {
    }

    D3D11CommandBuffer::D3D11CommandBuffer(D3D11Graphics* graphics)
        : CommandBuffer(graphics)
        , _isImmediate(false)
    {
        graphics->GetD3DDevice()->CreateDeferredContext1(0, _context.ReleaseAndGetAddressOf());
    }

    D3D11CommandBuffer::~D3D11CommandBuffer()
    {
        Destroy();
    }

    void D3D11CommandBuffer::Destroy()
    {
        if (!_isImmediate)
        {
            _context.Reset();
        }
    }

    void D3D11CommandBuffer::ResetState()
    {
        CommandBuffer::ResetState();

        _currentRenderPass = nullptr;
    }

    void D3D11CommandBuffer::Enqueue()
    {
        if (_isImmediate)
            return;
    }

    void D3D11CommandBuffer::Commit()
    {
        if (_isImmediate)
            return;

        Enqueue();

        ComPtr<ID3D11CommandList> commandList;
        _context->FinishCommandList(FALSE, commandList.ReleaseAndGetAddressOf());
    }

    void D3D11CommandBuffer::BeginRenderPass(RenderPass* renderPass, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil)
    {
        _currentRenderPass = static_cast<D3D11RenderPass*>(renderPass);
        _currentRenderPass->Bind(_context.Get());

        /*for (uint32_t i = 0; i < MaxColorAttachments; ++i)
        {
            const RenderPassColorAttachmentDescriptor& colorAttachment = descriptor.colorAttachments[i];
            Texture* texture = colorAttachment.texture;
            if (!texture)
                continue;

            D3D11Texture* d3dTexture = static_cast<D3D11Texture*>(texture);
            _boundRTV[_boundRTVCount] = d3dTexture->GetRenderTargetView(colorAttachment.level, colorAttachment.slice);

            switch (colorAttachment.loadAction)
            {
            case LoadAction::Clear:
                //_context->ClearRenderTargetView(
                //    d3dTexture->GetRTV(),
                //    colorAttachment.clearColor);
                break;

            default:
                break;
            }
        }*/

        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = 800;
        viewport.Height = 600;
        viewport.MinDepth = D3D11_MIN_DEPTH;
        viewport.MaxDepth = D3D11_MAX_DEPTH;
        D3D11_RECT scissorRect;
        scissorRect.left = 0;
        scissorRect.top = 0;
        scissorRect.right = 800;
        scissorRect.bottom = 600;

        _context->RSSetViewports(1, &viewport);
        _context->RSSetScissorRects(1, &scissorRect);
    }

    void D3D11CommandBuffer::EndRenderPass()
    {
        ID3D11RenderTargetView* nullViews[] = { nullptr };
        _context->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    }

    void D3D11CommandBuffer::SetPipeline(const SharedPtr<PipelineState>& pipeline)
    {
    }

    void D3D11CommandBuffer::OnSetVertexBuffer(GpuBuffer* buffer, uint32_t binding, uint64_t offset)
    {
    }

    void D3D11CommandBuffer::SetIndexBufferCore(GpuBuffer* buffer, uint32_t offset, IndexType indexType)
    {
    }

    void D3D11CommandBuffer::DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
    {
        if (!PrepareDraw(topology))
            return;
    }

    void D3D11CommandBuffer::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {
        if (!PrepareDraw(topology))
            return;
    }

    bool D3D11CommandBuffer::PrepareDraw(PrimitiveTopology topology)
    {
        return true;
    }
}
