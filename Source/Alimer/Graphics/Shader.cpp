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
#include "../IO/FileSystem.h"
#include "../Core/Log.h"
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

                pipelineResource.name = std::string(resource.name);
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

    ShaderModule::ShaderModule(GraphicsDevice* device, Util::Hash hash, const uint32_t* pCode, size_t size)
        : GraphicsResource(device)
        , _hash(hash)
    {
        // Reflection all shader resouces.
        SPIRVReflectResources(pCode, size, &_reflection);
    }

    Program::Program(GraphicsDevice* device, Util::Hash hash, const std::vector<ShaderModule*>& shaders)
        : GraphicsResource(device)
        , _hash(hash)
    {
    }

    ShaderTemplate::ShaderTemplate(const std::string &path)
        : _path(path)
    {
    }

    const ShaderTemplate::Variant* ShaderTemplate::RegisterVariant(const std::vector<std::pair<std::string, int>> *defines)
    {
        Util::Hasher h;
        if (defines)
        {
            for (auto &define : *defines)
            {
                h.u64(std::hash<std::string>()(define.first));
                h.u32(uint32_t(define.second));
            }
        }

        auto hash = h.get();
        auto *ret = _variants.Find(hash);
        if (!ret)
        {
            auto variant = std::make_unique<Variant>();

            String errorLog;
            if (!ShaderCompiler::Compile(_path, "main", variant->spirv, errorLog))
            {
                ALIMER_LOGCRITICALF("Shader compilation failed: %s", errorLog.CString());
                throw std::runtime_error("Shader compile failed.");
            }
            variant->instance++;
            if (defines)
                variant->defines = *defines;

            ret = _variants.Insert(hash, std::move(variant));
        }
        return ret;
    }

    ShaderProgram::ShaderProgram(GraphicsDevice* device)
        : GraphicsResource(device)
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

    void ShaderProgram::SetStage(ShaderStage stage, ShaderTemplate* shader)
    {
        _stages[static_cast<unsigned>(stage)] = shader;
        ALIMER_ASSERT(_variants.empty());
    }

    uint32_t ShaderProgram::RegisterVariant(const std::vector<std::pair<std::string, int>> &defines)
    {
        Util::Hasher h;
        for (auto &define : defines)
        {
            h.u64(std::hash<std::string>()(define.first));
            h.s32(define.second);
        }

        auto hash = h.get();

        auto itr = std::find(_variantHashes.begin(), _variantHashes.end(), hash);
        if (itr != _variantHashes.end())
        {
            uint32_t ret = uint32_t(itr - _variantHashes.begin());
            return ret;
        }

        uint32_t index = uint32_t(_variants.size());
        _variants.emplace_back();
        auto &var = _variants.back();
        _variantHashes.push_back(hash);

        for (unsigned i = 0; i < static_cast<unsigned>(ShaderStage::Count); i++)
        {
            if (_stages[i])
            {
                var.stages[i] = _stages[i]->RegisterVariant(&defines);
            }
        }

        // Make sure it's compiled correctly.
        GetVariant(index);
        return index;
    }

    Program* ShaderProgram::GetVariant(uint32_t variant)
    {
        auto &var = _variants[variant];
        auto *vert = var.stages[static_cast<unsigned>(ShaderStage::Vertex)];
        auto *frag = var.stages[static_cast<unsigned>(ShaderStage::Fragment)];
        auto *comp = var.stages[static_cast<unsigned>(ShaderStage::Compute)];
        if (comp)
        {
        }
        else if (vert && frag)
        {
            auto &vert_instance = var.shaderInstance[static_cast<unsigned>(ShaderStage::Vertex)];
            auto &frag_instance = var.shaderInstance[static_cast<unsigned>(ShaderStage::Fragment)];

            if (vert_instance != vert->instance || frag_instance != frag->instance)
            {
                if (vert_instance != vert->instance
                    || frag_instance != frag->instance)
                {
                    vert_instance = vert->instance;
                    frag_instance = frag->instance;
                    var.program = _device->RequestProgram(
                        vert->spirv.data(), vert->spirv.size() * sizeof(uint32_t),
                        frag->spirv.data(), frag->spirv.size() * sizeof(uint32_t));
                }

                return var.program;
            }
            else
            {
                return var.program;
            }
        }

        return nullptr;
    }
}
