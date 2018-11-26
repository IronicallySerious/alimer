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
#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/ShaderCompiler.h"
#include "../Resource/ResourceManager.h"
#include "../Resource/ResourceLoader.h"
#include "../IO/FileSystem.h"
#include "../Debug/Log.h"
#include <spirv-cross/spirv_glsl.hpp>

namespace Alimer
{
    class CustomCompiler : public spirv_cross::CompilerGLSL
    {
    public:
        CustomCompiler(const uint32_t *ir, size_t word_count)
            : spirv_cross::CompilerGLSL(ir, word_count)
        {
        }

        ParamAccess GetAccessFlags(const spirv_cross::SPIRType& type)
        {
            auto all_members_flag_mask = spirv_cross::Bitset(~0ULL);
            for (auto i = 0U; i < type.member_types.size(); ++i)
                all_members_flag_mask.merge_and(get_member_decoration_bitset(type.self, i));

            auto base_flags = ir.meta[type.self].decoration.decoration_flags;
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

    void SPIRVReflectResources(const uint32_t* pCode, size_t size, ShaderReflection* reflection)
    {
        ALIMER_ASSERT(reflection);

        // Parse SPIRV binary.
        CustomCompiler compiler(pCode, size / sizeof(uint32_t));
        spirv_cross::CompilerGLSL::Options opts = compiler.get_common_options();
        opts.enable_420pack_extension = true;
        compiler.set_common_options(opts);

        // Reflect on all resource bindings.
        auto resources = compiler.get_shader_resources();

        switch (compiler.get_execution_model())
        {
        case spv::ExecutionModelVertex:
            reflection->stage = ShaderStage::Vertex;
            break;
        case spv::ExecutionModelTessellationControl:
            reflection->stage = ShaderStage::TessControl;
            break;
        case spv::ExecutionModelTessellationEvaluation:
            reflection->stage = ShaderStage::TessEvaluation;
            break;
        case spv::ExecutionModelGeometry:
            reflection->stage = ShaderStage::Geometry;
            break;

        case spv::ExecutionModelFragment:
            reflection->stage = ShaderStage::Fragment;
            break;
        case spv::ExecutionModelGLCompute:
            reflection->stage = ShaderStage::Compute;
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

                pipelineResource.name = String(resource.name.c_str());
                shaderResources.push_back(pipelineResource);
            }
        };

        for (auto &attrib : resources.stage_inputs)
        {
            auto location = compiler.get_decoration(attrib.id, spv::DecorationLocation);
            reflection->inputMask |= 1u << location;
        }

        for (auto &attrib : resources.stage_outputs)
        {
            auto location = compiler.get_decoration(attrib.id, spv::DecorationLocation);
            reflection->outputMask |= 1u << location;
        }

        ExtractInputOutputs(reflection->stage, resources.stage_inputs, compiler, ResourceParamType::Input, ParamAccess::Read, reflection->resources);
        ExtractInputOutputs(reflection->stage, resources.stage_outputs, compiler, ResourceParamType::Output, ParamAccess::Write, reflection->resources);
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
        if (_handle != nullptr)
        {
            agpuDestroyShader(_handle);
            _handle = nullptr;
        }
    }

    bool Shader::Define(ShaderStage stage, const String& shaderSource, const String& entryPoint)
    {
        _shaderDescriptor.stages[static_cast<unsigned>(stage)].blob = agpuCompileShader(
            static_cast<AgpuShaderStage>(stage),
            shaderSource.CString(),
            entryPoint.CString()
        );
        return _shaderDescriptor.stages[static_cast<unsigned>(stage)].blob.size > 0;
    }

    bool Shader::Define(const  std::vector<uint8_t>& bytecode)
    {
        ShaderReflection reflection;
        SPIRVReflectResources(reinterpret_cast<const uint32_t*>(bytecode.data()), bytecode.size(), &reflection);

        _shaderDescriptor.stages[static_cast<unsigned>(reflection.stage)].blob.size = bytecode.size();
        _shaderDescriptor.stages[static_cast<unsigned>(reflection.stage)].blob.data = new uint8_t[bytecode.size()];
        memcpy(_shaderDescriptor.stages[static_cast<unsigned>(reflection.stage)].blob.data, bytecode.data(), bytecode.size());

        return false;
    }

    bool Shader::Finalize()
    {
        _handle = agpuCreateShader(&_shaderDescriptor);
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
        std::vector<uint8_t> _bytecode;
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
        shader->Define(_bytecode);
        return shader;
    }


    void Shader::RegisterObject()
    {
        RegisterFactory<Shader>();

        // Register loader.
        GetSubsystem<ResourceManager>()->AddLoader(new ShaderLoader());
    }
}
