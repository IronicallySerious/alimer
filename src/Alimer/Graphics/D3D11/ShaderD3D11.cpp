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

#include "ShaderD3D11.h"
#include "DeviceD3D11.h"
#include "../D3D/D3DShaderCompiler.h"
#include "../../Core/Log.h"
#include <spirv-cross/spirv_hlsl.hpp>
using namespace Microsoft::WRL;

namespace alimer
{
    ShaderD3D11::ShaderD3D11(DeviceD3D11* device, const PODVector<uint8_t>& bytecode)
        : _stage(ShaderStages::Compute)
    {
        device->GetD3DDevice()->CreateComputeShader(bytecode.Data(), bytecode.Size(), nullptr, _computeShader.ReleaseAndGetAddressOf());
    }

    ShaderD3D11::ShaderD3D11(DeviceD3D11* device, const PODVector<uint8_t>& vertex,
        const PODVector<uint8_t>& tessControl,
        const PODVector<uint8_t>& tessEval,
        const PODVector<uint8_t>& geometry,
        const PODVector<uint8_t>& fragment)
        : _stage(ShaderStages::None)
        , _vertexShaderBlob(vertex)
    {
        device->GetD3DDevice()->CreateVertexShader(vertex.Data(), vertex.Size(), nullptr, _vertexShader.ReleaseAndGetAddressOf());
        device->GetD3DDevice()->CreatePixelShader(fragment.Data(), fragment.Size(), nullptr, _pixelShader.ReleaseAndGetAddressOf());

        // Check if dx bycode.
        /*if (size > 4
            && data[0] == 'D'
            && data[1] == 'X'
            && data[2] == 'B'
            && data[3] == 'C')
        {
        }
        else
        {
            const uint32_t major = device->GetShaderModerMajor();
            const uint32_t minor = device->GetShaderModerMinor();

            spirv_cross::CompilerGLSL::Options options_glsl;
            options_glsl.vertex.fixup_clipspace = false;
            options_glsl.vertex.flip_vert_y = false;
            options_glsl.flatten_multidimensional_arrays = true;

            spirv_cross::CompilerHLSL compiler(reinterpret_cast<const uint32_t*>(data), size / 4);
            compiler.set_common_options(options_glsl);

            spirv_cross::CompilerHLSL::Options options_hlsl;
            options_hlsl.shader_model = major * 10 + minor;
            compiler.set_hlsl_options(options_hlsl);

            uint32_t new_builtin = compiler.remap_num_workgroups_builtin();
            if (new_builtin)
            {
                compiler.set_decoration(new_builtin, spv::DecorationDescriptorSet, 0);
                compiler.set_decoration(new_builtin, spv::DecorationBinding, 0);
            }

            std::string hlslSource = compiler.compile();
            auto d3dBytecode = D3DShaderCompiler::Compile(hlslSource, stage, "main");
            switch (stage)
            {
            case ShaderStages::Vertex:
            {
                ID3D11VertexShader* vertexShader;
                device->GetD3DDevice()->CreateVertexShader(d3dBytecode.Data(), d3dBytecode.Size(), nullptr, &vertexShader);
                _shader.Attach(vertexShader);
                break;
            }
            case ShaderStages::TessellationControl:
            {
                ID3D11HullShader* hullShader;
                device->GetD3DDevice()->CreateHullShader(d3dBytecode.Data(), d3dBytecode.Size(), nullptr, &hullShader);
                _shader.Attach(hullShader);
                break;
            }
            case ShaderStages::TessellationEvaluation:
            {
                ID3D11DomainShader* domainShader;
                device->GetD3DDevice()->CreateDomainShader(d3dBytecode.Data(), d3dBytecode.Size(), nullptr, &domainShader);
                _shader.Attach(domainShader);
                break;
            }
            case ShaderStages::Geometry:
            {
                ID3D11GeometryShader* geometryShader;
                device->GetD3DDevice()->CreateGeometryShader(d3dBytecode.Data(), d3dBytecode.Size(), nullptr, &geometryShader);
                _shader.Attach(geometryShader);
                break;
            }
            default:
                break;
            }
        }*/
    }

    ShaderD3D11::~ShaderD3D11()
    {
        if (any(_stage & ShaderStages::Compute))
        {
            ALIMER_ASSERT(_computeShader.Reset() == 0);
        }
        else
        {
            ALIMER_ASSERT(_vertexShader.Reset() == 0);
            ALIMER_ASSERT(_pixelShader.Reset() == 0);
        }
    }

#if TODO_D3D11
    ShaderD3D11::ShaderD3D11(DeviceD3D11* device, const char* source)
        : _device(device)
    {

        auto vertex = D3DShaderCompiler::Compile(source, ShaderStage::Vertex, "VSMain");
        auto fragment = D3DShaderCompiler::Compile(source, ShaderStage::Fragment, "PSMain");

        /*for (unsigned i = 0; i < static_cast<unsigned>(ShaderStage::Count); i++)
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
        }*/
    }

    void ShaderD3D11::Bind(ID3D11DeviceContext* context)
    {
        /*if (_computeShader != nullptr)
        {
            context->CSSetShader(_computeShader, nullptr, 0);
        }
        else
        {
            context->VSSetShader(_vertexShader, nullptr, 0);
            context->PSSetShader(_pixelShader, nullptr, 0);
        }*/
    }
#endif
}
