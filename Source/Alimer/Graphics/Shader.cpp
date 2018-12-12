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
#include "../Graphics/GraphicsImpl.h"
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
        , _impl(nullptr)
    {
    }

    ShaderModule::~ShaderModule()
    {
        Destroy();
    }

    void ShaderModule::Destroy()
    {
        SafeDelete(_impl);
    }

    bool ShaderModule::BeginLoad(Stream& source)
    {
        String extension = FileSystem::GetExtension(source.GetName());
        if (extension == ".vert")
            _stage = ShaderStage::Vertex;
        else if(extension == ".frag")
            _stage = ShaderStage::Compute;
        _sourceCode.Clear();
        return ProcessIncludes(_sourceCode, source);
    }

    bool ShaderModule::EndLoad()
    {
        return Define(_stage, _sourceCode, "main");
    }

    bool ShaderModule::ProcessIncludes(String& code, Stream& source)
    {
        ResourceManager* resources = GetSubsystem<ResourceManager>();

        while (!source.IsEof())
        {
            String line = source.ReadLine();

            if (line.StartsWith("#include"))
            {
                String includeFileName = FileSystem::GetPath(source.GetName()) + line.Substring(9).Replaced("\"", "").Trimmed();
                UniquePtr<Stream> includeStream = resources->OpenResource(includeFileName);
                if (includeStream.IsNull())
                    return false;

                // Add the include file into the current code recursively
                if (!ProcessIncludes(code, *includeStream))
                    return false;
            }
            else
            {
                code += line;
                code += "\n";
            }
        }

        // Finally insert an empty line to mark the space between files
        code += "\n";

        return true;
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
                _shaders[i] = nullptr;
            }
        }
    }

    bool Shader::BeginLoad(Stream& source)
    {
        auto bytecode = source.ReadBytes();
        ALIMER_UNUSED(bytecode);
        return false;
    }

    bool Shader::EndLoad()
    {
        return true;
    }

    bool Shader::Define(ShaderModule* vertex, ShaderModule* fragment)
    {
        ALIMER_ASSERT(vertex);
        ALIMER_ASSERT(fragment);

        _shaders[static_cast<unsigned>(ShaderStage::Vertex)] = vertex;
        _shaders[static_cast<unsigned>(ShaderStage::Fragment)] = fragment;
        return true;
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
