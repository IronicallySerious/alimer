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

#pragma once

#include "../Pipeline.h"
#include "BackendD3D11.h"
#include <unordered_map>

namespace alimer
{
    /// D3D11 Pipeline implementation.
    class PipelineD3D11 final : public PipelineHandle
    {
    public:
        PipelineD3D11(GraphicsDeviceD3D11* device, const RenderPipelineDescriptor* descriptor);
        ~PipelineD3D11() override;

        ID3D11BlendState*           GetBlendState() const { return _blendState; }
        ID3D11DepthStencilState*    GetDepthStencilState() const { return _depthStencilState; }
        ID3D11RasterizerState*      GetRasterizerState() const { return _rasterizerState; }
        ID3D11VertexShader*         GetVertexShader() const { return _vertexShader; }
        ID3D11PixelShader*          GetPixelShader() const { return _pixelShader; }
        ID3D11ComputeShader*        GetComputeShader() const { return _computeShader; }
        ID3D11InputLayout*          GetInputLayout() const { return _inputLayout; }

    private:
        ID3D11Device* _d3dDevice;

        ID3D11BlendState*           _blendState = nullptr;
        ID3D11DepthStencilState*    _depthStencilState = nullptr;
        ID3D11RasterizerState*      _rasterizerState = nullptr;
        ID3D11VertexShader*         _vertexShader = nullptr;
        ID3D11PixelShader*          _pixelShader = nullptr;
        ID3D11ComputeShader*        _computeShader = nullptr;
        ID3D11InputLayout*          _inputLayout = nullptr;
    };
}
