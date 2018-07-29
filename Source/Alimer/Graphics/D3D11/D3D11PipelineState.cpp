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

#include "D3D11PipelineState.h"
#include "D3D11Shader.h"
#include "D3D11Graphics.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11PipelineState::D3D11PipelineState(D3D11Graphics* graphics, const RenderPipelineDescription& description)
        : PipelineState(graphics, true)
    {
        _shader = StaticCast<D3D11Shader>(description.shader);
        HRESULT hr = S_OK;

        // Create blend state.
        D3D11_BLEND_DESC1 bsDesc;
        memset(&bsDesc, 0, sizeof(bsDesc));

        bsDesc.AlphaToCoverageEnable = FALSE;
        bsDesc.IndependentBlendEnable = FALSE;

        const D3D11_RENDER_TARGET_BLEND_DESC1 renderTargetBlendDesc =
        {
            FALSE,FALSE,
            D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
            D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
            D3D11_LOGIC_OP_NOOP,
            D3D11_COLOR_WRITE_ENABLE_ALL,
        };
        for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            bsDesc.RenderTarget[i] = renderTargetBlendDesc;
        }

        hr = graphics->GetD3DDevice()->CreateBlendState1(&bsDesc, &_d3d11BlendState);
        if (FAILED(hr))
        {
            ALIMER_LOGERROR("D3D11 - Failed to create BlendState");
        }

        // Create rasterizer state.
        D3D11_RASTERIZER_DESC1 rsDesc;
        memset(&rsDesc, 0, sizeof(rsDesc));
        rsDesc.FillMode = D3D11_FILL_SOLID;
        rsDesc.CullMode = D3D11_CULL_NONE;
        rsDesc.FrontCounterClockwise = FALSE;
        rsDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
        rsDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
        rsDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rsDesc.DepthClipEnable = TRUE;
        rsDesc.ScissorEnable = FALSE;
        rsDesc.MultisampleEnable = FALSE;
        rsDesc.AntialiasedLineEnable = FALSE;
        rsDesc.ForcedSampleCount = 0;
        hr = graphics->GetD3DDevice()->CreateRasterizerState1(&rsDesc, &_d3d11RasterizerState);
        if (FAILED(hr))
        {
            ALIMER_LOGERROR("D3D11 - Failed to create RasterizerState");
        }

        // Create depth-stencil state.
        D3D11_DEPTH_STENCIL_DESC dssDesc;
        memset(&dssDesc, 0, sizeof(dssDesc));

        dssDesc.DepthEnable = FALSE;
        dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dssDesc.DepthFunc = D3D11_COMPARISON_LESS;
        dssDesc.StencilEnable = FALSE;
        dssDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        dssDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

        const D3D11_DEPTH_STENCILOP_DESC defaultStencilOp =
        {
            D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP_KEEP,
            D3D11_COMPARISON_ALWAYS
        };

        dssDesc.FrontFace = defaultStencilOp;
        dssDesc.BackFace = defaultStencilOp;

        hr = graphics->GetD3DDevice()->CreateDepthStencilState(&dssDesc, &_d3d11DepthStencilState);
        if (FAILED(hr))
        {
            ALIMER_LOGERROR("D3D11 - Failed to create DepthStencilState");
        }
    }

    D3D11PipelineState::~D3D11PipelineState() = default;

    void D3D11PipelineState::Bind(ID3D11DeviceContext1* context)
    {
        if (_isGraphics)
        {
            _shader->Bind(context);
        }
        else
        {
        }
    }
}
