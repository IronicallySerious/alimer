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

#include "../Graphics/Shader.h"
#include "../Graphics/GPUDevice.h"
#include "../Graphics/GPUDeviceImpl.h"
#include "../Graphics/ShaderCompiler.h"
#include "../Resource/ResourceManager.h"
#include "../Resource/ResourceLoader.h"
#include "../IO/FileSystem.h"
#include "../Core/Log.h"
#include <spirv-cross/spirv_glsl.hpp>

namespace Alimer
{
    Shader::Shader()
        : GPUResource(GetSubsystem<GPUDevice>(), Type::Shader)
    {
        // Reflection all shader resouces.
        //SPIRVReflectResources(reinterpret_cast<const uint32_t*>(blob.data), blob.size, &_reflection);
    }

    Shader::~Shader()
    {
        Destroy();
    }

    void Shader::Destroy()
    {
        SafeDelete(_shader);
    }

    bool Shader::Define(const String& shaderSource)
    {
        Destroy();
        _shader = _device->GetImpl()->CreateShader(shaderSource.CString());
        return _shader != nullptr;
    }

    bool Shader::Define(ShaderModule* vertex, ShaderModule* fragment)
    {
        ALIMER_ASSERT(vertex);
        ALIMER_ASSERT(fragment);

        _shaders[static_cast<unsigned>(ShaderStage::Vertex)] = vertex;
        _shaders[static_cast<unsigned>(ShaderStage::Fragment)] = fragment;
        return true;
    }

    void Shader::RegisterObject()
    {
        RegisterFactory<Shader>();
    }
}
