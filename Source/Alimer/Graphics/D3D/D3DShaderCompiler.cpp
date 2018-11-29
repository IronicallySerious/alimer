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

#include "AlimerConfig.h"

#if ALIMER_COMPILE_D3D11 || ALIMER_COMPILE_D3D12
#include "D3DShaderCompiler.h"
#include "../../Base/String.h"
#include "../../Core/Log.h"

namespace Alimer
{
    ID3DBlob* D3DShaderCompiler::Compile(pD3DCompile d3dCompile, const char* source, size_t sourceLength, ShaderStage stage, uint32_t major, uint32_t minor)
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

        String compileTarget;
        switch (stage)
        {
        case ShaderStage::Vertex:
            compileTarget = String::Format("vs_%u_%u", major, minor);
            break;
        case ShaderStage::TessControl:
            compileTarget = String::Format("hs_%u_%u", major, minor);
            break;
        case ShaderStage::TessEvaluation:
            compileTarget = String::Format("ds_%u_%u", major, minor);
            break;
        case ShaderStage::Geometry:
            compileTarget = String::Format("gs_%u_%u", major, minor);
            break;
        case ShaderStage::Fragment:
            compileTarget = String::Format("ps_%u_%u", major, minor);
            break;
        case ShaderStage::Compute:
            compileTarget = String::Format("cs_%u_%u", major, minor);
            break;
        default:
            break;
        }

        ID3DBlob* shaderBlob;
        ID3DBlob* errorsBlob;
        if (FAILED(d3dCompile(
            source,
            sourceLength,
            nullptr,
            nullptr,
            nullptr,
            "main",
            compileTarget.CString(),
            compileFlags, 0,
            &shaderBlob,
            &errorsBlob)))
        {
            String errorMessage = String((const char*)errorsBlob->GetBufferPointer(), (uint32_t)errorsBlob->GetBufferSize() - 1);
            ALIMER_LOGERRORF("D3DCompile failed with error: %s", errorMessage.CString());
            return {};
        }

        SafeRelease(errorsBlob);

        return shaderBlob;
    }
}
#endif /* ALIMER_COMPILE_D3D11 || ALIMER_COMPILE_D3D12 */
