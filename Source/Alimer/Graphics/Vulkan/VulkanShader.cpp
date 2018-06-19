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

#include "VulkanShader.h"
#include "VulkanGraphics.h"
#include "VulkanPipelineLayout.h"
#include "VulkanConvert.h"

namespace Alimer
{
    static VkShaderModule CreateShaderModule(VkDevice device, const ShaderStageDescription& desc)
    {
        VkShaderModuleCreateInfo moduleCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        moduleCreateInfo.pNext = nullptr;
        moduleCreateInfo.flags = 0;
        moduleCreateInfo.codeSize = desc.code.size() * sizeof(uint32_t);
        moduleCreateInfo.pCode = desc.code.data();

        VkShaderModule shaderModule;
        vkThrowIfFailed(vkCreateShaderModule(
            device,
            &moduleCreateInfo,
            nullptr,
            &shaderModule));
        return shaderModule;
    }

    VulkanShader::VulkanShader(VulkanGraphics* graphics, const ShaderStageDescription& desc)
        : Shader()
        , _logicalDevice(graphics->GetLogicalDevice())
    {
        _shaderModules[static_cast<unsigned>(ShaderStage::Compute)] = CreateShaderModule(_logicalDevice, desc);
        _pipelineLayout = graphics->RequestPipelineLayout(_layout);
    }

    /// Constructor.
    VulkanShader::VulkanShader(VulkanGraphics* graphics, const ShaderStageDescription& vertex, const ShaderStageDescription& fragment)
        : Shader(vertex, fragment)
        , _logicalDevice(graphics->GetLogicalDevice())
    {
        _shaderModules[static_cast<unsigned>(ShaderStage::Vertex)] = CreateShaderModule(_logicalDevice, vertex);
        _shaderModules[static_cast<unsigned>(ShaderStage::Fragment)] = CreateShaderModule(_logicalDevice, fragment);

        _pipelineLayout = graphics->RequestPipelineLayout(_layout);
    }

    VulkanShader::~VulkanShader()
    {
        for (uint32_t i = 0; i < static_cast<unsigned>(ShaderStage::Count); i++)
        {
            if (_shaderModules[i] != VK_NULL_HANDLE)
            {
                vkDestroyShaderModule(_logicalDevice, _shaderModules[i], nullptr);
                _shaderModules[i] = VK_NULL_HANDLE;
            }
        }
    }

    VkShaderModule VulkanShader::GetVkShaderModule(ShaderStage stage) const
    {
        return _shaderModules[static_cast<unsigned>(stage)];
    }
}
