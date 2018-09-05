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
    static VkShaderModule CreateShaderModule(VkDevice device, const void *pCode, size_t codeSize)
    {
        VkShaderModuleCreateInfo moduleCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        moduleCreateInfo.pNext = nullptr;
        moduleCreateInfo.flags = 0;
        moduleCreateInfo.codeSize = codeSize;
        moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(pCode);

        VkShaderModule shaderModule;
        vkThrowIfFailed(vkCreateShaderModule(
            device,
            &moduleCreateInfo,
            nullptr,
            &shaderModule));
        return shaderModule;
    }

    VulkanShaderModule::VulkanShaderModule(VulkanGraphics* graphics, const std::vector<uint32_t>& spirv)
        : ShaderModule(graphics, spirv)
        , _logicalDevice(graphics->GetLogicalDevice())
    {
        VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.codeSize = spirv.size() * sizeof(uint32_t);
        createInfo.pCode = spirv.data();

        vkThrowIfFailed(vkCreateShaderModule(
            _logicalDevice,
            &createInfo,
            nullptr,
            &_handle));
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
    }

    VulkanShader::VulkanShader(VulkanGraphics* graphics, const ShaderProgramDescriptor* descriptor)
        : ShaderProgram(graphics, descriptor)
        , _logicalDevice(graphics->GetLogicalDevice())
    {
        //_shaderModules[static_cast<unsigned>(ShaderStage::Compute)] = CreateShaderModule(_logicalDevice, pCode, codeSize);
        //_pipelineLayout = graphics->RequestPipelineLayout(_layout);
    }

    /// Constructor.
    /*VulkanShader::VulkanShader(VulkanGraphics* graphics,
        const void *pVertexCode, size_t vertexCodeSize,
        const void *pFragmentCode, size_t fragmentCodeSize)
        : Shader(graphics, pVertexCode, vertexCodeSize, pFragmentCode, fragmentCodeSize)
        , _logicalDevice(graphics->GetLogicalDevice())
    {
        _shaderModules[static_cast<unsigned>(VulkanShaderStage::Vertex)] = CreateShaderModule(_logicalDevice, pVertexCode, vertexCodeSize);
        _shaderModules[static_cast<unsigned>(VulkanShaderStage::Fragment)] = CreateShaderModule(_logicalDevice, pFragmentCode, fragmentCodeSize);

        _pipelineLayout = graphics->RequestPipelineLayout(_layout);
    }*/

    VulkanShader::~VulkanShader()
    {
        Destroy();
    }

    void VulkanShader::Destroy()
    {
        for (uint32_t i = 0; i < static_cast<unsigned>(VulkanShaderStage::Count); i++)
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

    VkPipeline VulkanShader::GetGraphicsPipeline(Hash hash)
    {
        auto it = _graphicsPipelineCache.find(hash);
        if (it != _graphicsPipelineCache.end())
            return it->second;

        return VK_NULL_HANDLE;
    }

    void VulkanShader::AddPipeline(Hash hash, VkPipeline pipeline)
    {
        _graphicsPipelineCache[hash] = pipeline;
    }
}
