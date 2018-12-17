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

#include "../Graphics/ShaderModule.h"
#include "../Graphics/GPUDevice.h"
#include "../Graphics/ShaderCompiler.h"
#include "../IO/FileSystem.h"
#include "../Resource/ResourceManager.h"
#include "../Core/Log.h"

namespace Alimer
{
    ShaderModule::ShaderModule(GPUDevice* device)
        : GPUResource(device, Type::ShaderModule)
    {
    }

    ShaderModule::~ShaderModule()
    {
        Destroy();
    }

    void ShaderModule::Destroy()
    {
    }

    /*bool ShaderModule::BeginLoad(Stream& source)
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
    }*/

    bool ShaderModule::ProcessIncludes(String& code, Stream& source)
    {
        ResourceManager* resources = Object::GetSubsystem<ResourceManager>();

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

    void ShaderModule::RegisterObject()
    {
        //RegisterFactory<ShaderModule>();

        // Register loader.
        //GetSubsystem<ResourceManager>()->AddLoader(new ShaderLoader());
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
