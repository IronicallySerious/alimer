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

#include "Graphics/Shader.h"
#include "Graphics/Graphics.h"
#include <spirv_glsl.hpp>
#include <vector>
using namespace std;

namespace Alimer
{
    class CustomCompiler : public spirv_cross::CompilerGLSL
    {
    public:
        CustomCompiler(const std::vector<uint32_t>& spirv)
            : spirv_cross::CompilerGLSL(spirv)
        {

        }

        PipelineResourceAccess GetAccessFlags(const spirv_cross::SPIRType& type)
        {
            auto all_members_flag_mask = spirv_cross::Bitset(~0ULL);
            for (auto i = 0U; i < type.member_types.size(); ++i)
                all_members_flag_mask.merge_and(get_member_decoration_bitset(type.self, i));

            auto base_flags = meta[type.self].decoration.decoration_flags;
            base_flags.merge_or(spirv_cross::Bitset(all_members_flag_mask));

            PipelineResourceAccess access = PipelineResourceAccess::ReadWrite;
            if (base_flags.get(spv::DecorationNonReadable))
                access = PipelineResourceAccess::Write;
            else if (base_flags.get(spv::DecorationNonWritable))
                access = PipelineResourceAccess::Read;

            return access;
        }
    };

    static PipelineResourceBaseType SPIRVConvertBaseType(spirv_cross::SPIRType::BaseType type)
    {
        switch (type)
        {
        case spirv_cross::SPIRType::BaseType::Void:
            return PipelineResourceBaseType::Void;
        case spirv_cross::SPIRType::BaseType::Boolean:
            return PipelineResourceBaseType::Boolean;
        case spirv_cross::SPIRType::BaseType::Char:
            return PipelineResourceBaseType::Char;
        case spirv_cross::SPIRType::BaseType::Int:
            return PipelineResourceBaseType::Int;
        case spirv_cross::SPIRType::BaseType::UInt:
            return PipelineResourceBaseType::UInt;
        case spirv_cross::SPIRType::BaseType::Int64:
            return PipelineResourceBaseType::Int64;
        case spirv_cross::SPIRType::BaseType::UInt64:
            return PipelineResourceBaseType::UInt64;
        case spirv_cross::SPIRType::BaseType::Half:
            return PipelineResourceBaseType::Half;

        case spirv_cross::SPIRType::BaseType::Float:
            return PipelineResourceBaseType::Float;
        case spirv_cross::SPIRType::BaseType::Double:
            return PipelineResourceBaseType::Double;
        default:
            return PipelineResourceBaseType::Unknown;
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
            PipelineResourceType resourceType,
            PipelineResourceAccess access,
            std::vector<PipelineResource>& shaderResources)
        {
            for (const auto& resource : resources)
            {
                const auto& typeInfo = compiler.get_type_from_variable(resource.id);

                PipelineResource pipelineResource = {};
                pipelineResource.stages = stage;
                pipelineResource.baseType = SPIRVConvertBaseType(typeInfo.basetype);
                pipelineResource.resourceType = resourceType;
                pipelineResource.access = access;
                pipelineResource.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
                pipelineResource.vecSize = typeInfo.vecsize;
                pipelineResource.arraySize = (typeInfo.array.size() == 0) ? 1 : typeInfo.array[0];

                memcpy(pipelineResource.name, resource.name.c_str(), std::min(sizeof(pipelineResource.name), resource.name.length()));
                shaderResources.push_back(pipelineResource);
            }
        };

        ExtractInputOutputs(stage, resources.stage_inputs, compiler, PipelineResourceType::Input, PipelineResourceAccess::Read, shaderResources);
        ExtractInputOutputs(stage, resources.stage_outputs, compiler, PipelineResourceType::Output, PipelineResourceAccess::Write, shaderResources);
    }

    ShaderModule::ShaderModule(Graphics* graphics, const std::vector<uint32_t>& spirv)
        : GpuResource(graphics, GpuResourceType::ShaderModule)
        , _spirv(std::move(spirv))
    {
        // Reflection all shader resouces.
         SPIRVReflectResources(spirv, _stage, _resources);
    }

    Shader::Shader(Graphics* graphics, const void *pCode, size_t codeSize)
        : GpuResource(graphics, GpuResourceType::Pipeline)
        , _isCompute(true)
    {
        Reflect(ShaderStage::Compute, pCode, codeSize);

        for (uint32_t i = 0; i < MaxDescriptorSets; i++)
        {
            if (_layout.sets[i].stages != 0)
                _layout.descriptorSetMask |= 1u << i;
        }
    }

    Shader::Shader(Graphics* graphics,
        const void *pVertexCode, size_t vertexCodeSize,
        const void *pFragmentCode, size_t fragmentCodeSize)
        : GpuResource(graphics, GpuResourceType::Pipeline)
        , _isCompute(false)
    {
        Reflect(ShaderStage::Vertex, pVertexCode, vertexCodeSize);
        Reflect(ShaderStage::Fragment, pFragmentCode, fragmentCodeSize);

        for (uint32_t i = 0; i < MaxDescriptorSets; i++)
        {
            if (_layout.sets[i].stages != 0)
                _layout.descriptorSetMask |= 1u << i;
        }
    }

    Shader::~Shader()
    {
    }

    void Shader::Reflect(ShaderStage stage, const void *pCode, size_t codeSize)
    {
        spirv_cross::Compiler compiler(reinterpret_cast<const uint32_t*>(pCode), codeSize / sizeof(uint32_t));
        auto resources = compiler.get_shader_resources();

        if (stage == ShaderStage::Vertex)
        {
            for (auto &attrib : resources.stage_inputs)
            {
                uint32_t location = compiler.get_decoration(attrib.id, spv::DecorationLocation);

                if (location >= MaxVertexAttributes)
                {
                    ALIMER_LOGERROR("SPIRV attribute location higher than allowed one");
                    return;
                }

                _layout.attributeMask |= 1u << location;
            }
        }
        else if (stage == ShaderStage::Fragment)
        {
            for (auto &attrib : resources.stage_outputs)
            {
                uint32_t location = compiler.get_decoration(attrib.id, spv::DecorationLocation);
                _layout.renderTargetMask |= 1u << location;
            }
        }

        /*auto ExtractResourcesBinding = [this](
            const ShaderStage& stage,
            const std::vector<spirv_cross::Resource>& resources,
            const spirv_cross::Compiler& compiler,
            BindingType bindingType) {

            for (const auto& resource : resources) {
                auto bitSet = compiler.get_decoration_bitset(resource.id);
                ALIMER_ASSERT(bitSet.get(spv::DecorationBinding) && bitSet.get(spv::DecorationDescriptorSet));
                uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

                if (binding >= MaxBindingsPerSet || set >= MaxDescriptorSets) {
                    ALIMER_LOGERROR("SPIRV binding or set higher than allowed one");
                    continue;
                }

                switch (bindingType)
                {
                case BindingType::UniformBuffer:
                    _layout.sets[set].uniformBufferMask |= 1u << binding;
                    break;
                case BindingType::SampledTexture:
                    break;
                case BindingType::Sampler:
                    break;
                case BindingType::StorageBuffer:
                    break;
                default:
                    break;
                }

                _layout.sets[set].stages |= 1u << static_cast<unsigned>(stage);
            }
        };

        ExtractResourcesBinding(stage, resources.uniform_buffers, compiler, BindingType::UniformBuffer);
        ExtractResourcesBinding(stage, resources.separate_images, compiler, BindingType::SampledTexture);
        ExtractResourcesBinding(stage, resources.separate_samplers, compiler, BindingType::Sampler);
        ExtractResourcesBinding(stage, resources.storage_buffers, compiler, BindingType::StorageBuffer);*/
    }
}
