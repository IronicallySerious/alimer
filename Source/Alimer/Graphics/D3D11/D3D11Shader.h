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

#include "Graphics/Shader.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
	class D3D11Graphics;

	/// D3D11 Shader implementation.
	class D3D11Shader final : public Shader
	{
	public:
		/// Constructor.
        D3D11Shader(D3D11Graphics* graphics, const void *pCode, size_t codeSize);
		/// Constructor.
        D3D11Shader(D3D11Graphics* graphics,
            const void *pVertexCode, size_t vertexCodeSize,
            const void *pFragmentCode, size_t fragmentCodeSize);

		/// Destructor.
		~D3D11Shader() override;

        void Destroy() override;

        void Bind(ID3D11DeviceContext1* context);

        std::vector<uint8_t> AcquireVertexShaderBytecode();
        uint64_t GetVertexShaderHash() const { return _vertexShaderHash; }
        ID3D11VertexShader* GetD3DVertexShader() const { return _d3dVertexShader; }
        ID3D11PixelShader* GetD3DPixelShader() const { return _d3dPixelShader; }
        ID3D11ComputeShader* GetD3DComputeShader() const { return _d3dComputeShader; }

	private:
        ID3D11VertexShader* _d3dVertexShader = nullptr;
        ID3D11PixelShader* _d3dPixelShader = nullptr;
        ID3D11ComputeShader* _d3dComputeShader = nullptr;
        std::vector<uint8_t> _vsByteCode;
        uint64_t _vertexShaderHash = 0;
	};
}
