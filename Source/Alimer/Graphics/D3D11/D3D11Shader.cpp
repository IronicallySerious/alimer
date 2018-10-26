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
#include "D3D11GraphicsDevice.h"
#include "../D3D/D3DShaderCompiler.h"
#include "../D3D/D3DPlatformFunctions.h"
#include "../../Core/Log.h"
#include <spirv-cross/spirv_hlsl.hpp>
using namespace Microsoft::WRL;

namespace Alimer
{
    static ID3DBlob* ConvertAndCompileHLSL(
        D3D11GraphicsDevice* graphics,
        const ShaderBlob& blob,
        ShaderStage stage)
    {
        spirv_cross::CompilerGLSL::Options options_glsl;
        //options_glsl.vertex.fixup_clipspace = true;
        //options_glsl.vertex.flip_vert_y = true;

        spirv_cross::CompilerHLSL compiler(reinterpret_cast<const uint32_t*>(blob.data), blob.size / 4);
        compiler.set_common_options(options_glsl);

        const uint32_t major = graphics->GetShaderModerMajor();
        const uint32_t minor = graphics->GetShaderModerMinor();
        spirv_cross::CompilerHLSL::Options options_hlsl;
        options_hlsl.shader_model = major * 10 + minor;
        compiler.set_hlsl_options(options_hlsl);

        // Remap attributes with HLSL semantic.
        std::vector<spirv_cross::HLSLVertexAttributeRemap> remaps;
        for (int i = 0; i < static_cast<uint32_t>(VertexElementSemantic::Count); i++)
        {
            VertexElementSemantic semantic = static_cast<VertexElementSemantic>(i);
            spirv_cross::HLSLVertexAttributeRemap remap = { (uint32_t)i , EnumToString(semantic) };
            remaps.push_back(std::move(remap));
        }

        std::string hlslSource = compiler.compile(std::move(remaps));
        return D3DShaderCompiler::Compile(graphics->GetFunctions()->d3dCompile, hlslSource, stage, major, minor);
    }

    D3D11Shader::D3D11Shader(D3D11GraphicsDevice* device, const ShaderDescriptor* descriptor)
        : Shader(device, descriptor)
    {
        for (unsigned i = 0; i < static_cast<unsigned>(ShaderStage::Count); i++)
        {
            auto shaderBlob = descriptor->stages[i];
            if (shaderBlob.size == 0 || shaderBlob.data == nullptr)
                continue;

            ShaderStage stage = static_cast<ShaderStage>(i);
            ID3DBlob* blob = ConvertAndCompileHLSL(device, shaderBlob, stage);
            switch (stage)
            {
            case ShaderStage::Vertex:
            {
                _vsBlob = blob;
                device->GetD3DDevice()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &_vertexShader);
                break;
            }
            case ShaderStage::TessControl:
                break;
            case ShaderStage::TessEvaluation:
                break;
            case ShaderStage::Geometry:
                break;
            case ShaderStage::Fragment:
                device->GetD3DDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &_pixelShader);
                break;
            case ShaderStage::Compute:
                device->GetD3DDevice()->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &_computeShader);
                break;
            default:
                break;
            }

            if (stage != ShaderStage::Vertex)
            {
                blob->Release();
            }
        }
    }

    D3D11Shader::~D3D11Shader()
    {
        Destroy();
    }

    void D3D11Shader::Destroy()
    {
        if (_computeShader != nullptr)
        {
            SafeRelease(_computeShader, "ID3D11ComputeShader");
        }
        else
        {
            SafeRelease(_vertexShader, "ID3D11VertexShader");
            SafeRelease(_vsBlob, "ID3DBlob");
            SafeRelease(_pixelShader, "ID3D11PixelShader");
        }
    }

    void D3D11Shader::Bind(ID3D11DeviceContext* context)
    {
        if (_computeShader != nullptr)
        {
            context->CSSetShader(_computeShader, nullptr, 0);
        }
        else
        {
            context->VSSetShader(_vertexShader, nullptr, 0);
            context->PSSetShader(_pixelShader, nullptr, 0);
        }
    }
}
