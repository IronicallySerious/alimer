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

#pragma once

#include "../Shader.h"
#include "VulkanBackend.h"
#include "../../Base/HashMap.h"

namespace Alimer
{
    class VulkanGraphicsDevice;
    class VulkanPipelineLayout;

    /// Vulkan ShaderModule implementation.
    class VulkanShader final : public ShaderModule
    {
    public:
        /// Constructor.
        VulkanShader(VulkanGraphicsDevice* device, Util::Hash hash, const uint32_t* pCode, size_t size);
        ~VulkanShader() override;
        void Destroy() override;

        VkShaderModule GetHandle() const { return _handle; }
    private:
        VkDevice _logicalDevice;
        VkShaderModule _handle = VK_NULL_HANDLE;
    };


    struct VulkanResourceLayout
    {
        uint32_t vertexAttributeMask = 0;
        uint32_t renderTargetMask = 0;
        VkPushConstantRange ranges[static_cast<unsigned>(ShaderStage::Count)] = {};
        uint32_t numRanges = 0;
        uint32_t descriptorSetMask = 0;
    };

    class VulkanProgram final : public Program
    {
    public:
        /// Constructor.
        VulkanProgram(VulkanGraphicsDevice* device, Util::Hash hash, const std::vector<ShaderModule*>& stages);
        ~VulkanProgram() override;
        void Destroy() override;

        VkShaderModule GetVkShaderModule(unsigned stage) const { return _shaderModules[stage]; }
        VkShaderModule GetVkShaderModule(ShaderStage stage) const;
        VulkanPipelineLayout* GetPipelineLayout() const { return _pipelineLayout; }
        VkPipeline GetGraphicsPipeline(Util::Hash hash);
        void AddPipeline(Util::Hash hash, VkPipeline pipeline);

    private:
        VkDevice _logicalDevice;
        VkShaderModule _shaderModules[static_cast<unsigned>(ShaderStage::Count)] = {};
        VulkanPipelineLayout* _pipelineLayout = nullptr;
        Util::HashMap<VkPipeline> _graphicsPipelineCache;
    };
}
