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

#include "AlimerConfig.h"
#include "D3DShaderCompiler.h"
#include "../../Core/Log.h"
#include <d3dcompiler.h>
#include <fmt/printf.h>
using namespace std;

namespace alimer
{
    ShaderStageDescriptor D3DShaderCompiler::Compile(const string& source, ShaderStage stage, const string& entryPoint, uint32_t major, uint32_t minor)
    {
#if ALIMER_D3D_DYNAMIC_LIB
        static pD3DCompile D3DCompile = nullptr;
        if (!D3DCompile)
        {
            // TODO(tfoley): maybe want to search for one of a few versions of the DLL
            HMODULE compilerModule = LoadLibraryA("d3dcompiler_47.dll");
            if (!compilerModule)
            {
                ALIMER_LOGERROR("Failed load 'd3dcompiler_47.dll'");
                return {};
            }

            D3DCompile = (pD3DCompile)GetProcAddress(compilerModule, "D3DCompile");
            if (!D3DCompile)
            {
                ALIMER_LOGERROR("Failed load symbol 'D3DCompile' symbol");
                return {};
            }
        }
#endif /* ALIMER_D3D_DYNAMIC_LIB */

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
            compileTarget = fmt::sprintf("vs_%u_%u", major, minor);
            break;
        case ShaderStage::TessControl:
            compileTarget = fmt::sprintf("hs_%u_%u", major, minor);
            break;
        case ShaderStage::TessEvaluation:
            compileTarget = fmt::sprintf("ds_%u_%u", major, minor);
            break;
        case ShaderStage::Geometry:
            compileTarget = fmt::sprintf("gs_%u_%u", major, minor);
            break;
        case ShaderStage::Fragment:
            compileTarget = fmt::sprintf("ps_%u_%u", major, minor);
            break;
        case ShaderStage::Compute:
            compileTarget = fmt::sprintf("cs_%u_%u", major, minor);
            break;
        default:
            break;
        }

        ID3DBlob* shaderBlob;
        ID3DBlob* errorsBlob;
        if (FAILED(D3DCompile(
            source.c_str(),
            source.length(),
            nullptr,
            nullptr,
            nullptr,
            "main",
            compileTarget.c_str(),
            compileFlags, 0,
            &shaderBlob,
            &errorsBlob)))
        {
            string errorMessage = string((const char*)errorsBlob->GetBufferPointer(), (uint32_t)errorsBlob->GetBufferSize() - 1);
            ALIMER_LOGERROR("D3DCompile failed with error: {}", errorMessage);
            return {};
        }

        SafeRelease(errorsBlob);

        vector<uint8_t> blob(static_cast<uint32_t>(shaderBlob->GetBufferSize()));
        memcpy(blob.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
        return blob;
    }
}
