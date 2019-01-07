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

#include "VulkanGraphicsDevice.h"
#include "VulkanShader.h"
#include "VulkanPipelineLayout.h"
#include "VulkanConvert.h"

namespace Alimer
{
    /*VulkanShaderModule::VulkanShaderModule(VulkanGraphicsDevice* device, uint64_t hash, const ShaderBlob& blob)
        : ShaderModule(device, hash, blob)
        , _logicalDevice(device->GetDevice())
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.codeSize = blob.size;
        createInfo.pCode = reinterpret_cast<const uint32_t*>(blob.data);

        VkResult result = vkCreateShaderModule(
            _logicalDevice,
            &createInfo,
            nullptr,
            &_handle);

        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERRORF("[Vulkan] - Failed to create shader module.");
            return;
        }
    }

    VulkanShaderModule::~VulkanShaderModule()
    {
        Destroy();
    }

    void VulkanShaderModule::Destroy()
    {
        if (_handle != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(_logicalDevice, _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }
    }*/

    VulkanProgram::VulkanProgram(VulkanGraphicsDevice* device, uint64_t hash, const std::vector<ShaderModule*>& shaders)
        : Shader(device, nullptr)
        , _logicalDevice(device->GetDevice())
    {
        VulkanResourceLayout layout;
        layout.descriptorSetMask = 0;

        /*for (size_t i = 0, count = shaders.size(); i < count; ++i)
        {
            auto shader = static_cast<VulkanShaderModule*>(shaders[i]);
            if (shader->GetStage() == ShaderStage::Vertex)
            {
                layout.vertexAttributeMask = shader->GetReflection().inputMask;
            }
            if (shader->GetStage() == ShaderStage::Fragment)
            {
                layout.renderTargetMask = shader->GetReflection().outputMask;
            }

            _shaderModules[static_cast<unsigned>(shaders[i]->GetStage())] = shader->GetHandle();
        }

        _pipelineLayout = device->RequestPipelineLayout(&layout);*/
    }

    VulkanProgram::~VulkanProgram()
    {
        Destroy();
    }

    void VulkanProgram::Destroy()
    {
    }

    VkShaderModule VulkanProgram::GetVkShaderModule(ShaderStage stage) const
    {
        return _shaderModules[static_cast<unsigned>(stage)];
    }

    VkPipeline VulkanProgram::GetGraphicsPipeline(uint64_t hash)
    {
        auto it = _graphicsPipelineCache.find(hash);
        if (it != _graphicsPipelineCache.end())
            return it->second;

        return VK_NULL_HANDLE;
    }

    void VulkanProgram::AddPipeline(uint64_t hash, VkPipeline pipeline)
    {
        _graphicsPipelineCache[hash] = pipeline;
    }
}
