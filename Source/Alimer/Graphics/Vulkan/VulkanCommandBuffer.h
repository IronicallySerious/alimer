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

#include "../CommandBuffer.h"
#include "VulkanPrerequisites.h"
#include <vector>

namespace Alimer
{
    class VulkanGraphics;
    class VulkanCommandBuffer;
    class VulkanCommandQueue;
    class VulkanPipelineLayout;
    class VulkanPipelineState;
    class VulkanRenderPass;

    /// Vulkan CommandBuffer.
    class VulkanCommandBuffer final : public CommandBuffer
    {
    public:
        VulkanCommandBuffer(VulkanGraphics* graphics, VkCommandPool commandPool, bool secondary);
        ~VulkanCommandBuffer() override;

        void Begin(VkCommandBufferInheritanceInfo* inheritanceInfo);
        void End();

        void BeginRenderPassCore(RenderPass* renderPass, const Rectangle& renderArea, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil) override;
        void EndRenderPassCore() override;

        void SetViewport(const Viewport& viewport) override;
        void SetViewports(uint32_t numViewports, const Viewport* viewports) override;

        void SetScissor(const Rectangle& scissor) override;
        void SetScissors(uint32_t numScissors, const Rectangle* scissors) override;

        void SetPipeline(PipelineState* pipeline) override;

        void DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance) override;
        //void DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex) override;

        void ExecuteCommandsCore(uint32_t commandBufferCount, CommandBuffer* const* commandBuffers);

        void SetVertexBufferCore(GpuBuffer* buffer, uint32_t binding, uint64_t offset) override;
        //void SetIndexBufferCore(GpuBuffer* buffer, uint32_t offset, IndexType indexType) override;
        void SetUniformBufferCore(uint32_t set, uint32_t binding, const GpuBuffer* buffer, uint64_t offset, uint64_t range) override;

        VkCommandBuffer GetVkCommandBuffer() const { return _vkCommandBuffer; }
        bool IsSecondary() const { return _secondary; }

    private:
        void BeginCompute();
        void BeginGraphics();
        void BeginContext();
        bool PrepareDraw(PrimitiveTopology topology);

        void FlushDescriptorSet(uint32_t set);
        void FlushDescriptorSets();

    private:
        VulkanGraphics * _graphics;
        VkDevice _logicalDevice;
        VkCommandPool _commandPool;
        VkCommandBuffer _vkCommandBuffer;
        bool _secondary = false;
        VulkanRenderPass* _currentRenderPass = nullptr;
        VulkanPipelineState* _currentPipeline = nullptr;
        VulkanPipelineLayout* _currentPipelineLayout = nullptr;
        VkPipeline _currentVkPipeline = VK_NULL_HANDLE;
        VkPipelineLayout _currentVkPipelineLayout = VK_NULL_HANDLE;

        VkViewport _currentViewports[MaxViewportsAndScissors] = {};

        struct VertexBindingState
        {
            VkBuffer buffers[MaxVertexBufferBindings];
            uint64_t offsets[MaxVertexBufferBindings];
            uint64_t strides[MaxVertexBufferBindings];
        };

        struct IndexState
        {
            GpuBuffer* buffer;
            uint32_t offset;
            IndexType indexType;
        };

        struct ResourceBinding
        {
            union {
                VkDescriptorBufferInfo buffer;
                struct
                {
                    VkDescriptorImageInfo fp;
                    VkDescriptorImageInfo integer;
                } image;
                VkBufferView bufferView;
            };
        };

        struct ResourceBindings
        {
            ResourceBinding bindings[MaxDescriptorSets][MaxBindingsPerSet];
            uint8_t push_constant_data[MaxDescriptorSets];
        };

        VertexBindingState _vbo = {};
        IndexState _indexState = {};
        ResourceBindings _bindings;

        bool _isCompute = true;
        uint32_t _dirtyVbos = 0;
        uint32_t _dirtySets = 0;
    };
}
