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
#include "VulkanPrerequisites.h"
#include "../../Base/HashMap.h"
#include <vector>

namespace Alimer
{
    class VulkanGraphics;
    class VulkanPipelineLayout;

    enum class VulkanShaderStage
    {
        Vertex = 0,
        TessControl = 1,
        TessEvaluation = 2,
        Geometry = 3,
        Fragment = 4,
        Compute = 5,
        Count
    };

    /// Vulkan ShaderModule implementation.
    class VulkanShaderModule final : public ShaderModule
    {
    public:
        /// Constructor.
        VulkanShaderModule(VulkanGraphics* graphics, const std::vector<uint32_t>& spirv);
        ~VulkanShaderModule() override;

        void Destroy() override;

        VkShaderModule GetHandle() const { return _handle; }

    private:
        VkDevice _logicalDevice;
        VkShaderModule _handle = VK_NULL_HANDLE;
    };

    class VulkanShader final : public ShaderProgram
    {
    public:
        /// Constructor.
        VulkanShader(VulkanGraphics* graphics, const ShaderProgramDescriptor* descriptor);

        ~VulkanShader() override;
        void Destroy() override;

        VkShaderModule GetVkShaderModule(unsigned stage) const { return _shaderModules[stage]; }
        VkShaderModule GetVkShaderModule(ShaderStage stage) const;
        VulkanPipelineLayout* GetPipelineLayout() const { return _pipelineLayout; }
        VkPipeline GetGraphicsPipeline(Util::Hash hash);
        void AddPipeline(Util::Hash hash, VkPipeline pipeline);

    private:
        VkDevice _logicalDevice;
        VkShaderModule _shaderModules[static_cast<unsigned>(VulkanShaderStage::Count)] = {};
        VulkanPipelineLayout* _pipelineLayout = nullptr;
        Util::HashMap<VkPipeline> _graphicsPipelineCache;
    };
}
