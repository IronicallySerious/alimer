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

#include "D3DShaderCompiler.h"
#include "../../Core/String.h"
#include "../../Core/Log.h"
#include <d3dcompiler.h>

namespace Alimer
{
    namespace D3DShaderCompiler
    {
        std::vector<uint8_t> Compile(const std::string& hlslSource, ShaderStage stage, uint32_t major, uint32_t minor)
        {
            UINT compileFlags = 0;
#if defined(_DEBUG)
            // Enable better shader debugging with the graphics debugging tools.
            compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
            // Optimize.
            compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
            // SPRIV-cross does matrix multiplication expecting row major matrices
            compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

            std::string compileTarget;
            switch (stage)
            {
            case ShaderStage::Vertex:
                compileTarget = fmt::format("vs_{}_{}", major, minor);
                break;
            case ShaderStage::TessControl:
                compileTarget = fmt::format("hs_{}_{}", major, minor);
                break;
            case ShaderStage::TessEvaluation:
                compileTarget = fmt::format("ds_{}_{}", major, minor);
                break;
            case ShaderStage::Geometry:
                compileTarget = fmt::format("gs_{}_{}", major, minor);
                break;
            case ShaderStage::Fragment:
                compileTarget = fmt::format("ps_{}_{}", major, minor);
                break;
            case ShaderStage::Compute:
                compileTarget = fmt::format("cs_{}_{}", major, minor);
                break;
            default:
                break;
            }

            Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
            Microsoft::WRL::ComPtr<ID3DBlob> errors;
            if (FAILED(D3DCompile(
                hlslSource.c_str(),
                hlslSource.length(),
                nullptr,
                nullptr,
                nullptr,
                "main",
                compileTarget.c_str(),
                compileFlags, 0,
                shaderBlob.ReleaseAndGetAddressOf(),
                errors.ReleaseAndGetAddressOf())))
            {
                ALIMER_LOGERROR("D3DCompile failed with error: {}", reinterpret_cast<char*>(errors->GetBufferPointer()));
                return {};
            }

            std::vector<uint8_t> byteCode(shaderBlob->GetBufferSize());
            memcpy(byteCode.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
            return byteCode;
        }
    }
}
