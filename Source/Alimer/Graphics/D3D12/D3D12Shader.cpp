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

#include "D3D12Shader.h"
#include "D3D12Graphics.h"
#include "../../Core/Log.h"

namespace Alimer
{
	D3D12Shader::D3D12Shader(D3D12Graphics* graphics, ID3DBlob* blob)
		: Shader(graphics)
	{
		_byteCode.resize(blob->GetBufferSize());
		memcpy(_byteCode.data(), blob->GetBufferPointer(), blob->GetBufferSize());

		ID3D12ShaderReflection* reflection;

		HRESULT hr = D3DReflect(
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			_uuidof(ID3D12ShaderReflection),
			(void**)&reflection);

		if (FAILED(hr))
		{
			ALIMER_LOGERROR("Cannot reflect D3D compiled shader.");
			return;
		}

		D3D12_SHADER_DESC shaderDesc;
		hr = reflection->GetDesc(&shaderDesc);
		if (FAILED(hr))
		{
			ALIMER_LOGERROR("Cannot get D3D compiled shader desc.");
			return;
		}

		switch (D3D12_SHVER_GET_TYPE(shaderDesc.Version))
		{
			case D3D12_SHVER_VERTEX_SHADER:
				_stage = ShaderStage::Vertex;
				break;

			case D3D12_SHVER_PIXEL_SHADER:
				_stage = ShaderStage::Fragment;
				break;
		}

		reflection->Release();
	}

	D3D12Shader::~D3D12Shader()
	{
	}

	std::vector<uint8_t> D3D12Shader::AcquireBytecode()
	{
		return std::move(_byteCode);
	}
}
