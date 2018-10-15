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
#include "../Graphics/ShaderCompiler.h"
#include "../Graphics/Graphics.h"
#include "../IO/FileSystem.h"
#include "../Core/Log.h"
#include <spirv_glsl.hpp>
#include <vector>

namespace Alimer
{
    class CustomCompiler : public spirv_cross::CompilerGLSL
    {
    public:
        CustomCompiler(const std::vector<uint32_t>& spirv)
            : spirv_cross::CompilerGLSL(spirv)
        {
        }

        ParamAccess GetAccessFlags(const spirv_cross::SPIRType& type)
        {
            auto all_members_flag_mask = spirv_cross::Bitset(~0ULL);
            for (auto i = 0U; i < type.member_types.size(); ++i)
                all_members_flag_mask.merge_and(get_member_decoration_bitset(type.self, i));

            auto base_flags = meta[type.self].decoration.decoration_flags;
            base_flags.merge_or(spirv_cross::Bitset(all_members_flag_mask));

            ParamAccess access = ParamAccess::ReadWrite;
            if (base_flags.get(spv::DecorationNonReadable))
                access = ParamAccess::Write;
            else if (base_flags.get(spv::DecorationNonWritable))
                access = ParamAccess::Read;

            return access;
        }
    };

    static ParamDataType SPIRVConvertBaseType(spirv_cross::SPIRType::BaseType type)
    {
        switch (type)
        {
        case spirv_cross::SPIRType::BaseType::Void:
            return ParamDataType::Void;
        case spirv_cross::SPIRType::BaseType::Boolean:
            return ParamDataType::Boolean;
        case spirv_cross::SPIRType::BaseType::Char:
            return ParamDataType::Char;
        case spirv_cross::SPIRType::BaseType::Int:
            return ParamDataType::Int;
        case spirv_cross::SPIRType::BaseType::UInt:
            return ParamDataType::UInt;
        case spirv_cross::SPIRType::BaseType::Int64:
            return ParamDataType::Int64;
        case spirv_cross::SPIRType::BaseType::UInt64:
            return ParamDataType::UInt64;
        case spirv_cross::SPIRType::BaseType::Half:
            return ParamDataType::Half;
        case spirv_cross::SPIRType::BaseType::Float:
            return ParamDataType::Float;
        case spirv_cross::SPIRType::BaseType::Double:
            return ParamDataType::Double;
        case spirv_cross::SPIRType::BaseType::Struct:
            return ParamDataType::Struct;
        default:
            return ParamDataType::Unknown;
        }
    }

    void SPIRVReflectResources(const std::vector<uint32_t>& spirv, ShaderStage& stage, std::vector<PipelineResource>& shaderResources)
    {
        // Parse SPIRV binary.
        CustomCompiler compiler(spirv);
        spirv_cross::CompilerGLSL::Options opts = compiler.get_common_options();
        opts.enable_420pack_extension = true;
        compiler.set_common_options(opts);

        // Reflect on all resource bindings.
        auto resources = compiler.get_shader_resources();

        switch (compiler.get_execution_model())
        {
        case spv::ExecutionModelVertex:
            stage = ShaderStage::Vertex;
            break;
        case spv::ExecutionModelTessellationControl:
            stage = ShaderStage::TessControl;
            break;
        case spv::ExecutionModelTessellationEvaluation:
            stage = ShaderStage::TessEvaluation;
            break;
        case spv::ExecutionModelGeometry:
            stage = ShaderStage::Geometry;
            break;

        case spv::ExecutionModelFragment:
            stage = ShaderStage::Fragment;
            break;
        case spv::ExecutionModelGLCompute:
            stage = ShaderStage::Compute;
            break;
        default:
            ALIMER_LOGCRITICAL("Invalid shader execution model");
        }

        auto ExtractInputOutputs = [](
            const ShaderStage& stage,
            const std::vector<spirv_cross::Resource>& resources,
            const spirv_cross::Compiler& compiler,
            ResourceParamType resourceType,
            ParamAccess access,
            std::vector<PipelineResource>& shaderResources)
        {
            uint32_t stageMask = 1u << static_cast<uint32_t>(stage);

            for (const auto& resource : resources)
            {
                const auto& spirType = compiler.get_type_from_variable(resource.id);

                PipelineResource pipelineResource = {};
                pipelineResource.stages = static_cast<ShaderStageUsage>(stageMask);
                pipelineResource.dataType = SPIRVConvertBaseType(spirType.basetype);
                pipelineResource.resourceType = resourceType;
                pipelineResource.access = access;
                pipelineResource.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
                pipelineResource.vecSize = spirType.vecsize;
                pipelineResource.arraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];

                pipelineResource.name = std::string(resource.name);
                shaderResources.push_back(pipelineResource);
            }
        };

        ExtractInputOutputs(stage, resources.stage_inputs, compiler, ResourceParamType::Input, ParamAccess::Read, shaderResources);
        ExtractInputOutputs(stage, resources.stage_outputs, compiler, ResourceParamType::Output, ParamAccess::Write, shaderResources);
    }

    Shader::Shader()
    {
    }

    Shader::~Shader()
    {
        Destroy();
    }

    bool Shader::Define(ShaderStage stage, const String& url)
    {
        auto stream = FileSystem::Get().Open(url);
        if (!stream)
        {
            _infoLog = String::Format("Shader file '%s' does not exists", url.CString());
            return false;
        }

        _source = stream->ReadAllText();
        std::vector<uint32_t> spirv;
        if (!ShaderCompiler::Compile(_stage, _source, "main", spirv, stream->GetName(), _infoLog))
        {
            ALIMER_LOGERRORF("Shader compilation failed: %s", _infoLog.CString());
            return false;
        }

        return Define(stage, spirv);
    }

    bool Shader::Define(ShaderStage stage, const std::vector<uint32_t>& spirv)
    {
        _stage = stage;
        _spirv = std::move(spirv);

        // Reflection all shader resouces.
        SPIRVReflectResources(spirv, _stage, _resources);

        return BackendCreate();
    }

    std::vector<uint32_t> Shader::AcquireBytecode()
    {
        return std::move(_spirv);
    }

    ShaderProgram::ShaderProgram()
        : GpuResource(Object::GetSubsystem<Graphics>(), GpuResourceType::ShaderProgram)
        , _isCompute(false)
    {
        /*if (descriptor->stageCount == 1
            && descriptor->stages[0].module->GetStage() == ShaderStage::Compute)
        {
            _isCompute = true;
        }

        for (uint32_t i = 0; i < descriptor->stageCount; ++i)
        {
            auto stage = descriptor->stages[i];
            _shaders[static_cast<uint32_t>(stage.module->GetStage())] = stage.module;
        }*/
    }

    ShaderProgram::~ShaderProgram()
    {

    }

    bool ShaderProgram::Define(const String& vertexShaderUrl, const String& fragmentShaderUrl)
    {
        _shaders[static_cast<uint32_t>(ShaderStage::Vertex)].Define(ShaderStage::Vertex, vertexShaderUrl);
        _shaders[static_cast<uint32_t>(ShaderStage::Fragment)].Define(ShaderStage::Fragment, fragmentShaderUrl);
        return BackendCreate();
    }
}
