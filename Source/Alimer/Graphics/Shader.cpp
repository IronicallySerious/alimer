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
#include "../Graphics/Graphics.h"
#include "../Graphics/ShaderCompiler.h"
#include "../Resource/ResourceManager.h"
#include "../Resource/ResourceLoader.h"
#include "../IO/FileSystem.h"
#include "../Debug/Log.h"
#include <spirv-cross/spirv_glsl.hpp>

namespace Alimer
{
    ShaderModule::ShaderModule()
        : Resource()
    {
    }

    ShaderModule::~ShaderModule()
    {
        Destroy();
    }

    void ShaderModule::Destroy()
    {
    }

    Shader::Shader()
        : Resource()
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
        for (uint32_t i = 0; i < static_cast<unsigned>(ShaderStage::Count); ++i)
        {
            if (_shaders[i] != nullptr)
            {
                agpuDestroyShaderModule(_shaders[i]);
                _shaders[i] = nullptr;
            }
        }
    }

    bool Shader::Define(ShaderStage stage, const String& shaderSource, const String& entryPoint)
    {
        AgpuShaderModuleDescriptor descriptor = {};
        descriptor.stage = static_cast<AgpuShaderStageFlagBits>(1u << static_cast<unsigned>(stage));
        descriptor.source = shaderSource.CString();
        descriptor.entryPoint = entryPoint.CString();

        _shaders[static_cast<unsigned>(stage)] = agpuCreateShaderModule(&descriptor);
        return _shaders[static_cast<unsigned>(stage)] != nullptr;
    }

    bool Shader::Define(ShaderStage stage, const Vector<uint8_t>& bytecode)
    {
        AgpuShaderModuleDescriptor descriptor = {};
        descriptor.codeSize = bytecode.Size();
        descriptor.pCode = bytecode.Data();

        AgpuShaderModule shaderModule = agpuCreateShaderModule(&descriptor);

        //ShaderReflection reflection;
        //SPIRVReflectResources(reinterpret_cast<const uint32_t*>(bytecode.data()), bytecode.size(), &reflection);

        _shaders[static_cast<unsigned>(stage)] = shaderModule;
        return shaderModule != nullptr;
    }

    bool Shader::Finalize()
    {
        AgpuShaderStageDescriptor stages[static_cast<unsigned>(ShaderStage::Count)];
        uint32_t stageCount = 0;

        for (unsigned i = 0; i < static_cast<unsigned>(ShaderStage::Count); i++)
        {
            if (_shaders[i] != nullptr)
            {
                auto &stage = stages[stageCount++];
                stage.shaderModule = _shaders[i];
                stage.entryPoint = "main";
            }
        }

        AgpuShaderDescriptor descriptor = {};
        descriptor.stageCount = stageCount;
        descriptor.stages = stages;

        _handle = agpuCreateShader(&descriptor);
        if (_handle != nullptr)
        {
            return true;
        }

        return false;
    }

    class ShaderLoader final : public ResourceLoader
    {
    public:
        StringHash GetType() const override;

        bool BeginLoad(Stream& source) override;
        Object* EndLoad() override;

    private:
        Vector<uint8_t> _bytecode;
    };

    StringHash ShaderLoader::GetType() const
    {
        return Shader::GetTypeStatic();
    }

    bool ShaderLoader::BeginLoad(Stream& source)
    {
        _bytecode = source.ReadBytes();
        return true;
    }

    Object* ShaderLoader::EndLoad()
    {
        Shader* shader = new Shader();
        shader->Define(ShaderStage::Vertex, _bytecode);
        return shader;
    }

    void ShaderModule::RegisterObject()
    {
        RegisterFactory<ShaderModule>();

        // Register loader.
        //GetSubsystem<ResourceManager>()->AddLoader(new ShaderLoader());
    }

    void Shader::RegisterObject()
    {
        RegisterFactory<Shader>();

        // Register loader.
        GetSubsystem<ResourceManager>()->AddLoader(new ShaderLoader());
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
