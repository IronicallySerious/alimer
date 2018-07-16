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
#include <spirv_cross.hpp>
#include <vector>
using namespace std;

namespace Alimer
{
    Shader::Shader(Graphics* graphics, const void *pCode, size_t codeSize)
        : GpuResource(graphics, GpuResourceType::Shader)
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
        : GpuResource(graphics, GpuResourceType::Shader)
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

        auto ExtractResourcesBinding = [this](
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
        ExtractResourcesBinding(stage, resources.storage_buffers, compiler, BindingType::StorageBuffer);
    }
}
