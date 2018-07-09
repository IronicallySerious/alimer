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
#include "../../Core/Log.h"
#include <spirv-cross/spirv_hlsl.hpp>
using namespace Microsoft::WRL;

namespace Alimer
{
    static std::vector<uint8_t> ConvertAndCompileHLSL(
        const ShaderStageDescription& desc,
        ShaderStage stage,
        uint32_t major, uint32_t minor)
    {
        spirv_cross::CompilerGLSL::Options options_glsl;
        //options_glsl.vertex.fixup_clipspace = true;
        //options_glsl.vertex.flip_vert_y = true;

        spirv_cross::CompilerHLSL compiler(desc.pCode, desc.codeSize);
        compiler.set_common_options(options_glsl);

        spirv_cross::CompilerHLSL::Options options_hlsl;
        options_hlsl.shader_model = major * 10 + minor;
        compiler.set_hlsl_options(options_hlsl);

        std::string hlslSource = compiler.compile();
        return D3DShaderCompiler::Compile(hlslSource, stage, major, minor);
    }

    D3D11Shader::D3D11Shader(D3D11Graphics* graphics, const ShaderStageDescription& desc)
        : Shader()
        , _graphics(graphics)
    {
        _shaders[static_cast<unsigned>(ShaderStage::Compute)] = ConvertAndCompileHLSL(
            desc,
            ShaderStage::Compute,
            graphics->GetShaderModerMajor(),
            graphics->GetShaderModerMinor());
    }

    D3D11Shader::D3D11Shader(D3D11Graphics* graphics, const ShaderStageDescription& vertex, const ShaderStageDescription& fragment)
        : Shader(vertex, fragment)
        , _graphics(graphics)
    {
        const D3D_FEATURE_LEVEL featureLevel = graphics->GetFeatureLevel();

        _shaders[static_cast<unsigned>(ShaderStage::Vertex)] = ConvertAndCompileHLSL(
            vertex, ShaderStage::Vertex,
            graphics->GetShaderModerMajor(), graphics->GetShaderModerMinor()
        );
        _shaders[static_cast<unsigned>(ShaderStage::Fragment)] = ConvertAndCompileHLSL(
            fragment, ShaderStage::Fragment,
            graphics->GetShaderModerMajor(), graphics->GetShaderModerMinor()
        );
    }

    D3D11Shader::~D3D11Shader()
    {
    }

    bool D3D11Shader::HasBytecode(ShaderStage stage)
    {
        return _shaders[static_cast<unsigned>(stage)].size() > 0;
    }

    std::vector<uint8_t> D3D11Shader::AcquireBytecode(ShaderStage stage)
    {
        return std::move(_shaders[static_cast<unsigned>(stage)]);
    }
}
