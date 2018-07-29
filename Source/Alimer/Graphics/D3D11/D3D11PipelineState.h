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

#pragma once

#include "../PipelineState.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
	class D3D11Graphics;
    class D3D11Shader;

	/// D3D11 PipelineState implementation.
	class D3D11PipelineState final : public PipelineState
	{
	public:
		/// Constructor.
        D3D11PipelineState(D3D11Graphics* graphics, const RenderPipelineDescription& description);

		/// Destructor.
		~D3D11PipelineState() override;

        void Bind(ID3D11DeviceContext1* context);

        D3D11Shader* GetShader() const { return _shader.Get(); }
        ID3D11RasterizerState1* GetD3DRasterizerState() const { return _d3d11RasterizerState; }
        ID3D11DepthStencilState* GetD3DDepthStencilState() const { return _d3d11DepthStencilState; }
        ID3D11BlendState1* GetD3DBlendState() const { return _d3d11BlendState; }

	private:
        SharedPtr<D3D11Shader> _shader;
        ID3D11RasterizerState1* _d3d11RasterizerState = nullptr;
        ID3D11DepthStencilState* _d3d11DepthStencilState = nullptr;
        ID3D11BlendState1* _d3d11BlendState = nullptr;
	};
}
