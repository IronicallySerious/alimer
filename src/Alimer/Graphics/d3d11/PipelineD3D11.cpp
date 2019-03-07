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

#include "PipelineD3D11.h"
#include "ShaderD3D11.h"
#include "GraphicsDeviceD3D11.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace alimer
{
    PipelineD3D11::PipelineD3D11(GraphicsDeviceD3D11* device, const RenderPipelineDescriptor* descriptor)
        : _d3dDevice(device->GetD3DDevice())
    {
        auto &cache = device->GetCache();
        _blendState = cache.GetBlendState(D3D11_BLEND_ONE, D3D11_BLEND_ZERO);
        _vertexShader = static_cast<ShaderD3D11*>(descriptor->vertexShader->GetHandle())->GetHandle<ID3D11VertexShader>();
        _pixelShader = static_cast<ShaderD3D11*>(descriptor->fragmentShader->GetHandle())->GetHandle<ID3D11PixelShader>();
        /*for (uint32_t i = 0; i < static_cast<unsigned>(ShaderStage::Count); i++)
        {
            auto shaderModule = descriptor->shaders[i];
            if (shaderModule == nullptr)
                continue;

            ShaderStage stage = static_cast<ShaderStage>(i);
            switch (stage)
            {
            case ShaderStage::Vertex:
                _vertexShader = (ID3D11VertexShader*)cache.GetShader(shaderModule, vsBytecode);
                break;
            case ShaderStage::TessControl:
                break;
            case ShaderStage::TessEvaluation:
                break;
            case ShaderStage::Geometry:
                break;
            case ShaderStage::Fragment:
                _pixelShader = (ID3D11PixelShader*)cache.GetShader(shaderModule);
                break;
            case ShaderStage::Compute:
                _computeShader = (ID3D11ComputeShader*)cache.GetShader(shaderModule);
                break;
            default:
                break;
            }
        }*/

        auto blob = static_cast<ShaderD3D11*>(descriptor->vertexShader->GetHandle())->GetBlob();
        _inputLayout = cache.GetInputLayout(blob, descriptor);
    }

    PipelineD3D11::~PipelineD3D11()
    {
    }
}
