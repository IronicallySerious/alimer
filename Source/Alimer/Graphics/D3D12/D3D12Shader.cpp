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
#include <spirv-cross/spirv_hlsl.hpp>

namespace Alimer
{
    static std::vector<uint8_t> CompileHLSL(const std::string& hlslSource, ShaderStage stage)
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

        const char* compileTarget = nullptr;
        switch (stage)
        {
            case ShaderStage::Vertex:
                compileTarget = "vs_5_1";
                break;
            case ShaderStage::TessControl:
                compileTarget = "hs_5_1";
                break;
            case ShaderStage::TessEvaluation:
                compileTarget = "ds_5_1";
                break;
            case ShaderStage::Geometry:
                compileTarget = "gs_5_1";
                break;
            case ShaderStage::Fragment:
                compileTarget = "ps_5_1";
                break;
            case ShaderStage::Compute:
                compileTarget = "cs_5_1";
                break;
            default:
                break;
        }

        static decltype(D3DCompile)* d3dCompile_;
        static decltype(D3DStripShader)* d3dStripShader_;
        static decltype(D3DReflect)* d3dReflect_;

#if !ALIMER_PLATFORM_UWP
        if (!d3dCompile_)
        {
            HMODULE d3dCompilerLib = LoadLibraryW(D3DCOMPILER_DLL_W);
            d3dCompile_ = (decltype(d3dCompile_))::GetProcAddress(d3dCompilerLib, "D3DCompile");
            ALIMER_ASSERT(d3dCompile_);
            d3dStripShader_ = (decltype(d3dStripShader_))::GetProcAddress(d3dCompilerLib, "D3DStripShader");
            ALIMER_ASSERT(d3dStripShader_);
            d3dReflect_ = (decltype(d3dReflect_))::GetProcAddress(d3dCompilerLib, "D3DReflect");
            ALIMER_ASSERT(d3dReflect_);
        }
#else
        d3dCompile_ = D3DCompile;
        d3dStripShader_ = D3DStripShader;
        d3dReflect_ = D3DReflect;
#endif

        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errors;
        if (FAILED(d3dCompile_(
            hlslSource.c_str(),
            hlslSource.length(),
            nullptr,
            nullptr,
            nullptr,
            "main",
            compileTarget,
            compileFlags, 0,
            shaderBlob.ReleaseAndGetAddressOf(),
            errors.ReleaseAndGetAddressOf())))
        {
            ALIMER_LOGERROR("D3DCompile failed with error: %s", reinterpret_cast<char*>(errors->GetBufferPointer()));
            return {};
        }

        std::vector<uint8_t> byteCode(shaderBlob->GetBufferSize());
        memcpy(byteCode.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
        return byteCode;
    }

    static std::vector<uint8_t> ConvertAndCompileHLSL(const ShaderStageDescription& desc, ShaderStage stage)
    {
        spirv_cross::CompilerGLSL::Options options_glsl;
        //options_glsl.vertex.fixup_clipspace = true;
        //options_glsl.vertex.flip_vert_y = true;

        spirv_cross::CompilerHLSL compiler(desc.byteCode, desc.byteCodeSize);
        compiler.set_common_options(options_glsl);

        spirv_cross::CompilerHLSL::Options options_hlsl;
        options_hlsl.shader_model = 51;
        compiler.set_hlsl_options(options_hlsl);

        std::string hlslSource = compiler.compile();
        return CompileHLSL(hlslSource, stage);
    }

    D3D12Shader::D3D12Shader(D3D12Graphics* graphics, const ShaderStageDescription& desc)
        : Shader()
        , _graphics(graphics)
    {
        _shaders[static_cast<unsigned>(ShaderStage::Compute)] = ConvertAndCompileHLSL(desc, ShaderStage::Compute);
        InitializeRootSignature();
    }

    D3D12Shader::D3D12Shader(D3D12Graphics* graphics, const ShaderStageDescription& vertex, const ShaderStageDescription& fragment)
        : Shader()
        , _graphics(graphics)
    {
        _shaders[static_cast<unsigned>(ShaderStage::Vertex)] = ConvertAndCompileHLSL(vertex, ShaderStage::Vertex);
        _shaders[static_cast<unsigned>(ShaderStage::Fragment)] = ConvertAndCompileHLSL(fragment, ShaderStage::Fragment);
        InitializeRootSignature();
    }

    D3D12Shader::~D3D12Shader()
    {
    }

    void D3D12Shader::InitializeRootSignature()
    {
        // TODO: Cache root signature.
        // Allow input layout and deny uneccessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        CD3DX12_ROOT_PARAMETER1 rootParameters[1];

        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(
            1,
            rootParameters,
            0, nullptr,
            rootSignatureFlags);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        if (FAILED(AlimerD3DX12SerializeVersionedRootSignature(
            &rootSignatureDesc,
            _graphics->GetFeatureDataRootSignature().HighestVersion,
            &signature, &error)))
        {
            ALIMER_LOGERROR("D3D12SerializeRootSignature failed");
            return;
        }

        if (FAILED(
            _graphics->GetD3DDevice()->CreateRootSignature(
                0,
                signature->GetBufferPointer(),
                signature->GetBufferSize(),
                IID_PPV_ARGS(&_rootSignature))))
        {
            ALIMER_LOGERROR("CreateRootSignature failed");
            return;
        }
    }

    bool D3D12Shader::HasBytecode(ShaderStage stage)
    {
        return _shaders[static_cast<unsigned>(stage)].size() > 0;
    }

    std::vector<uint8_t> D3D12Shader::AcquireBytecode(ShaderStage stage)
    {
        return std::move(_shaders[static_cast<unsigned>(stage)]);
    }
}
