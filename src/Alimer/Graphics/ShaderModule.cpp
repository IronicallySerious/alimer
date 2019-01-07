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

#include "../Graphics/ShaderModule.h"
#include "../Graphics/GPUDevice.h"
#include "../Graphics/ShaderCompiler.h"
#include "../IO/FileSystem.h"
#include "../Resource/ResourceManager.h"
#include "../Core/Log.h"

namespace alimer
{
    ShaderModule::ShaderModule(GPUDevice* device)
        : GPUResource(device, Type::Shader)
    {
    }

    ShaderModule::~ShaderModule()
    {
        Destroy();
    }

    void ShaderModule::Destroy()
    {
    }

    bool ShaderModule::Define(ShaderStage stage, const String& shaderSource, const String& entryPoint)
    {
        _stage = stage;
        Destroy();

        return true;
    }

    bool ShaderModule::Define(ShaderStage stage, const Vector<uint8_t>& bytecode)
    {
        _stage = stage;
        return true;
    }

    void ShaderModule::RegisterObject()
    {
        //RegisterFactory<ShaderModule>();
    }

    const char* EnumToString(ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vertex: return "vertex";
        case ShaderStage::TessControl: return "tess_control";
        case ShaderStage::TessEvaluation: return "tess_eval";
        case ShaderStage::Geometry: return "geometry";
        case ShaderStage::Fragment: return "fragment";
        case ShaderStage::Compute: return "compute";
        default: return nullptr;
        }
    }
}
