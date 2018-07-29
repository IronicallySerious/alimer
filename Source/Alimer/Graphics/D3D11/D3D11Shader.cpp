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

#include "D3D11Shader.h"
#include "D3D11Graphics.h"
#include "../D3D/D3DShaderCompiler.h"
#include "../../Util/HashMap.h"
#include "../../Core/Log.h"
#include <spirv_hlsl.hpp>
using namespace Microsoft::WRL;

namespace Alimer
{
    static std::vector<uint8_t> ConvertAndCompileHLSL(
        const void *pCode, size_t codeSize,
        ShaderStage stage,
        uint32_t major, uint32_t minor)
    {
        spirv_cross::CompilerGLSL::Options options_glsl;
        //options_glsl.vertex.fixup_clipspace = true;
        //options_glsl.vertex.flip_vert_y = true;

        spirv_cross::CompilerHLSL compiler(reinterpret_cast<const uint32_t*>(pCode), codeSize / sizeof(uint32_t));
        compiler.set_common_options(options_glsl);

        spirv_cross::CompilerHLSL::Options options_hlsl;
        options_hlsl.shader_model = major * 10 + minor;
        compiler.set_hlsl_options(options_hlsl);

        std::string hlslSource = compiler.compile();
        return D3DShaderCompiler::Compile(hlslSource, stage, major, minor);
    }

    D3D11Shader::D3D11Shader(D3D11Graphics* graphics, const void *pCode, size_t codeSize)
        : Shader(graphics, pCode, codeSize)
    {
        auto byteCode = ConvertAndCompileHLSL(
            pCode, codeSize,
            ShaderStage::Compute,
            graphics->GetShaderModerMajor(),
            graphics->GetShaderModerMinor());

        graphics->GetD3DDevice()->CreateComputeShader(byteCode.data(), byteCode.size(), nullptr, &_d3dComputeShader);
    }

    D3D11Shader::D3D11Shader(D3D11Graphics* graphics,
        const void *pVertexCode, size_t vertexCodeSize,
        const void *pFragmentCode, size_t fragmentCodeSize)
        : Shader(graphics, pVertexCode, vertexCodeSize, pFragmentCode, fragmentCodeSize)
    {
        const D3D_FEATURE_LEVEL featureLevel = graphics->GetFeatureLevel();

        _vsByteCode = ConvertAndCompileHLSL(
            pVertexCode, vertexCodeSize, ShaderStage::Vertex,
            graphics->GetShaderModerMajor(), graphics->GetShaderModerMinor()
        );

        auto fsByteCode = ConvertAndCompileHLSL(
            pFragmentCode, fragmentCodeSize, ShaderStage::Fragment,
            graphics->GetShaderModerMajor(), graphics->GetShaderModerMinor()
        );

        Hasher h;
        h.data(_vsByteCode.data(), _vsByteCode.size());
        _vertexShaderHash = h.get();
        graphics->GetD3DDevice()->CreateVertexShader(_vsByteCode.data(), _vsByteCode.size(), nullptr, &_d3dVertexShader);
        graphics->GetD3DDevice()->CreatePixelShader(fsByteCode.data(), fsByteCode.size(), nullptr, &_d3dPixelShader);
    }

    D3D11Shader::~D3D11Shader()
    {
        Destroy();
    }

    void D3D11Shader::Destroy()
    {
        if (_isCompute)
        {
            if (_d3dComputeShader)
            {
#if defined(_DEBUG)
                ULONG refCount = GetRefCount(_d3dComputeShader);
                ALIMER_ASSERT_MSG(refCount == 1, "D3D11Shader leakage");
#endif

                _d3dComputeShader->Release();
                _d3dComputeShader = nullptr;
            }
        }
        else
        {
            if (_d3dVertexShader)
            {
#if defined(_DEBUG)
                ULONG refCount = GetRefCount(_d3dVertexShader);
                ALIMER_ASSERT_MSG(refCount == 1, "D3D11Shader leakage");
#endif

                _d3dVertexShader->Release();
                _d3dVertexShader = nullptr;
            }

            if (_d3dPixelShader)
            {
#if defined(_DEBUG)
                ULONG refCount = GetRefCount(_d3dPixelShader);
                ALIMER_ASSERT_MSG(refCount == 1, "D3D11Shader leakage");
#endif

                _d3dPixelShader->Release();
                _d3dPixelShader = nullptr;
            }
        }
    }

    void D3D11Shader::Bind(ID3D11DeviceContext1* context)
    {
        if (_isCompute)
        {
            context->CSSetShader(_d3dComputeShader, nullptr, 0);
        }
        else
        {
            context->VSSetShader(_d3dVertexShader, nullptr, 0);
            context->PSSetShader(_d3dPixelShader, nullptr, 0);
        }
    }

    std::vector<uint8_t> D3D11Shader::AcquireVertexShaderBytecode()
    {
        return std::move(_vsByteCode);
    }
}
