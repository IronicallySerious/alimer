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

#include "../Shader.h"
#include "BackendD3D11.h"

namespace alimer
{
    /// D3D11 Shader implementation.
    class ShaderD3D11 final : public ShaderHandle
    {
    public:
        /// Constructor.
        ShaderD3D11(GraphicsDeviceD3D11* device, ShaderStage stage, const std::string& code, const std::string& entryPoint);

        /// Destructor.
        ~ShaderD3D11() override;

        ID3D11VertexShader* GetVertexShader() const { return _vertexShader; }
        ID3D11HullShader* GetTessControlShader() const { return _tessControlShader; }
        ID3D11DomainShader* GetTessEvalShader() const { return _tessEvalShader; }
        ID3D11GeometryShader* GetGeometryShader() const { return _geometryShader; }
        ID3D11PixelShader* GetPixelShader() const { return _pixelShader; }
        ID3D11ComputeShader* GetComputeShader() const { return _computeShader; }

        const std::vector<uint8_t>& GetVertexShaderBlob() const { return _vertexShaderBlob; }

    private:
        ID3D11VertexShader* _vertexShader = nullptr;
        ID3D11HullShader* _tessControlShader = nullptr;
        ID3D11DomainShader* _tessEvalShader = nullptr;
        ID3D11GeometryShader* _geometryShader = nullptr;
        ID3D11PixelShader* _pixelShader = nullptr;
        ID3D11ComputeShader* _computeShader = nullptr;
        std::vector<uint8_t> _vertexShaderBlob;
    };
}
