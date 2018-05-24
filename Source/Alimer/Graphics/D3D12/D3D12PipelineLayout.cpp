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

#include "D3D12PipelineLayout.h"
#include "D3D12Graphics.h"
#include "../../Core/Log.h"

namespace Alimer
{
#if !ALIMER_PLATFORM_UWP
	extern PFN_D3D12_SERIALIZE_ROOT_SIGNATURE D3D12SerializeRootSignature;
#endif

	D3D12PipelineLayout::D3D12PipelineLayout(D3D12Graphics* graphics)
		: PipelineLayout(graphics)
	{
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.NumParameters = 0;
		rootSignatureDesc.pParameters = nullptr;
		rootSignatureDesc.NumStaticSamplers = 0;
		rootSignatureDesc.pStaticSamplers = nullptr;
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)))
		{
			ALIMER_LOGERROR("D3D12SerializeRootSignature failed");
			return;
		}

		if (FAILED(
			graphics->GetD3DDevice()->CreateRootSignature(
				0,
				signature->GetBufferPointer(),
				signature->GetBufferSize(),
				IID_PPV_ARGS(&_rootSignature))))
		{
			ALIMER_LOGERROR("CreateRootSignature failed");
			return;
		}
	}

	D3D12PipelineLayout::~D3D12PipelineLayout() = default;
}
