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

#include "D3D11CommandEncoder.h"
#include "D3D11CommandBuffer.h"
#include "D3D11Graphics.h"
#include "D3D11Texture.h"
#include "D3D11RenderPass.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11RenderPassCommandEncoder::D3D11RenderPassCommandEncoder(CommandBuffer* commandBuffer, ID3D11DeviceContext1* context)
        : RenderPassCommandEncoder(commandBuffer)
        , _context(context)
    {
    }

    D3D11RenderPassCommandEncoder::~D3D11RenderPassCommandEncoder()
    {

    }

    void D3D11RenderPassCommandEncoder::BeginRenderPass(RenderPass* renderPass, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil)
    {
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
    }

    void D3D11RenderPassCommandEncoder::Close()
    {
        ID3D11RenderTargetView* nullViews[] = { nullptr };
        _context->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
        _currentRenderPass = nullptr;
    }
}
