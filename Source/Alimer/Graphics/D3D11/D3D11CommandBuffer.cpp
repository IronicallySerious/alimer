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
#include "../D3D/D3DConvert.h"
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
    D3D11CommandBuffer::D3D11CommandBuffer(D3D11Graphics* graphics, ID3D11DeviceContext1* context)
        : CommandBuffer(graphics)
        , _context(context)
        , _isImmediate(true)
    {
    }

    D3D11CommandBuffer::~D3D11CommandBuffer()
    {
        Reset();
        Destroy();
    }

    void D3D11CommandBuffer::Destroy()
    {
        if (!_isImmediate)
        {
            _context->Release();
        }
    }

    void D3D11CommandBuffer::Reset()
    {
        _state = CommandBufferState::Ready;

        if (!_isImmediate)
        {
            _currentTopology = PrimitiveTopology::Count;
            _context->ClearState();
        }
    }

    void D3D11CommandBuffer::CommitCore()
    {
        if (_isImmediate)
            return;

        ComPtr<ID3D11CommandList> commandList;
        _context->FinishCommandList(FALSE, commandList.ReleaseAndGetAddressOf());
    }

    void D3D11CommandBuffer::BeginRenderPassCore(RenderPass* renderPass, const Rectangle& renderArea, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil)
    {
        if (!renderPass)
        {
            renderPass = StaticCast<D3D11Graphics>(_graphics)->GetBackbufferRenderPass();
        }

        _currentRenderPass = static_cast<D3D11RenderPass*>(renderPass);
        _currentRenderPass->Bind(_context);

        for (uint32_t i = 0; i < renderPass->GetColorAttachmentsCount(); ++i)
        {
            const RenderPassAttachment& colorAttachment = renderPass->GetColorAttachment(i);

            switch (colorAttachment.loadAction)
            {
            case LoadAction::Clear:
                _context->ClearRenderTargetView(
                    _currentRenderPass->GetRTV(i),
                    &clearColors[i].r);
                break;

            default:
                break;
            }
        }

        // Set viewport and scissor from render area.
        Rectangle setRenderArea = renderArea;
        if (renderArea.IsEmpty())
        {
            setRenderArea = Rectangle((int32_t)renderPass->GetWidth(), (int32_t)renderPass->GetHeight());
        }

        SetViewport(Viewport(setRenderArea));
        SetScissor(setRenderArea);
    }

    void D3D11CommandBuffer::EndRenderPassCore()
    {
        ID3D11RenderTargetView* nullViews[] = { nullptr };
        _context->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
        _currentRenderPass = nullptr;
    }

    void D3D11CommandBuffer::SetViewport(const Viewport& viewport)
    {
        _context->RSSetViewports(1, (D3D11_VIEWPORT*)&viewport);
    }

    void D3D11CommandBuffer::SetViewports(uint32_t numViewports, const Viewport* viewports)
    {
        _context->RSSetViewports(numViewports, (D3D11_VIEWPORT*)viewports);
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

    void D3D11CommandBuffer::SetScissors(uint32_t numScissors, const Rectangle* scissors)
    {
        numScissors = std::min(numScissors, static_cast<uint32_t>(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));
        D3D11_RECT scissorsD3D[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for (uint32_t i = 0; i < numScissors; ++i)
        {
            const Rectangle& src = scissors[i];
            D3D11_RECT& dst = scissorsD3D[i];

            dst.left = src.x;
            dst.top = src.y;
            dst.right = src.x + src.width;
            dst.bottom = src.y + src.height;
        }

        _context->RSSetScissorRects(numScissors, scissorsD3D);
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

    bool D3D11CommandBuffer::PrepareDraw(PrimitiveTopology topology)
    {
        if (_currentTopology != topology)
        {
            _context->IASetPrimitiveTopology(d3d::Convert(topology));
            _currentTopology = topology;
        }

        return false;
    }
}
