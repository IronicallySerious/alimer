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
    class VulkanShaderModule final : public ShaderModule
    {
    public:
        /// Constructor.
        VulkanShaderModule(VulkanGraphicsDevice* device, uint64_t hash, const ShaderBlob& blob);
        ~VulkanShaderModule() override;
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

    class VulkanProgram final : public Shader
    {
    public:
        /// Constructor.
        VulkanProgram(VulkanGraphicsDevice* device, uint64_t hash, const std::vector<ShaderModule*>& stages);
        ~VulkanProgram() override;
        void Destroy();

        VkShaderModule GetVkShaderModule(unsigned stage) const { return _shaderModules[stage]; }
        VkShaderModule GetVkShaderModule(ShaderStage stage) const;
        VulkanPipelineLayout* GetPipelineLayout() const { return _pipelineLayout; }
        VkPipeline GetGraphicsPipeline(uint64_t hash);
        void AddPipeline(uint64_t hash, VkPipeline pipeline);

    private:
        VkDevice _logicalDevice;
        VkShaderModule _shaderModules[static_cast<unsigned>(ShaderStage::Count)] = {};
        VulkanPipelineLayout* _pipelineLayout = nullptr;
        HashMap<VkPipeline> _graphicsPipelineCache;
    };
}
