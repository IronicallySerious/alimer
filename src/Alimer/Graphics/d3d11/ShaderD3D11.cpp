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
#include "GraphicsDeviceD3D11.h"
#include "../D3D/D3DShaderCompiler.h"
#include "../../Core/Log.h"
#include <spirv_hlsl.hpp>
using namespace Microsoft::WRL;

namespace alimer
{
    ShaderD3D11::ShaderD3D11(GraphicsDeviceD3D11* device, const ShaderDescriptor* descriptor)
        : Shader(device, descriptor)
    {
        if (_compute)
        {
            const ShaderStageDescriptor& shaderStage = descriptor->stages[(unsigned)ShaderStage::Compute];
            device->GetD3DDevice()->CreateComputeShader(shaderStage.code, shaderStage.codeSize, nullptr, _computeShader.ReleaseAndGetAddressOf());
        }
        else
        {
        }
    }

    ShaderD3D11::~ShaderD3D11()
    {
        Destroy();
    }

    void ShaderD3D11::Destroy()
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
