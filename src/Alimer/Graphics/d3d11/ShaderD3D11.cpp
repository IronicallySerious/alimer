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
    ShaderD3D11::ShaderD3D11(GraphicsDeviceD3D11* device, ShaderStage stage, const std::string& code, const std::string& entryPoint)
    {
        auto shaderStage = D3DShaderCompiler::Compile(code, stage, entryPoint, device->GetShaderModerMajor(), device->GetShaderModerMinor());
        switch (stage)
        {
        case ShaderStage::Vertex:
        {
            device->GetD3DDevice()->CreateVertexShader(shaderStage.code, shaderStage.codeSize, nullptr, &_vertexShader);
            break;
        }
        case ShaderStage::TessControl:
            break;
        case ShaderStage::TessEvaluation:
            break;
        case ShaderStage::Geometry:
            break;
        case ShaderStage::Fragment:
            device->GetD3DDevice()->CreatePixelShader(shaderStage.code, shaderStage.codeSize, nullptr, &_pixelShader);
            break;
        case ShaderStage::Compute:
            device->GetD3DDevice()->CreateComputeShader(shaderStage.code, shaderStage.codeSize, nullptr, &_computeShader);
            break;
        default:
            break;
        }
    }

    ShaderD3D11::~ShaderD3D11()
    {
        SafeRelease(_vertexShader);
        SafeRelease(_tessControlShader);
        SafeRelease(_tessEvalShader);
        SafeRelease(_geometryShader);
        SafeRelease(_pixelShader);
        SafeRelease(_computeShader);
    }
}
